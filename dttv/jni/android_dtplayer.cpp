#ifndef NELEM
#define NELEM(x) ((int)(sizeof(x) / sizeof((x)[0])))
#endif
#define LOG_TAG "DTPLAYER-JNI"

#include "android_runtime/AndroidRuntime.h"  
#include "android_os_Parcel.h"
#include "android_util_Binder.h"
#include <binder/Parcel.h>
#include <jni.h>
#include <utils/Mutex.h>
#include <android/log.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "android_dtplayer.h"
#include "dtp_native_api.h"


// ------------------------------------------------------------
//OPENGL ESV2

#ifdef USE_OPENGL_V2

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#endif

#include "dt_lock.h"

//-----------------------------------------------------------

#define RGB565(r, g, b)  (((r) << (5+6)) | ((g) << 6) | (b))
#define GLRENDER_STATUS_IDLE 0
#define GLRENDER_STATUS_RUNNING 1
#define GLRENDER_STATUS_QUIT 2

#ifdef USE_OPENGL_V2
//For OPENGLESV2
enum {  
    ATTRIB_VERTEX,  
    ATTRIB_TEXTURE,  
}; 
typedef struct{
    int g_width;
    int g_height;

    GLuint g_texYId;
    GLuint g_texUId;
    GLuint g_texVId;
    GLuint simpleProgram;

    dt_av_frame_t frame;

	int status;
    int invalid_frame;
    int vertex_index;
    dt_lock_t mutex;
}gles2_ctx_t;

static gles2_ctx_t gl_ctx;

#endif

// ------------------------------------------------------------


// ------------------------------------------------------------

using namespace android;

// ------------------------------------------------------------

//------------------------------------------------------------
//jni code
struct fields_t {
    jfieldID    context;
    jfieldID    surface_texture;

    jmethodID   post_event;

    jmethodID   proxyConfigGetHost;
    jmethodID   proxyConfigGetPort;
    jmethodID   proxyConfigGetExclusionList;
};
static fields_t fields;
dt_lock_t sLock;
// ----------------------------------------------------------------------------

static const char * const kClassName = "dttv/app/DtPlayer";
static DTPlayer *dtPlayer = NULL; // player handle

#ifdef ENABLE_DTAP
extern "C" int dtap_change_effect(int effect_id);
#endif

//================================================

static JavaVM *gvm = NULL;
static jclass mClass = NULL;
static jmethodID notify_cb = NULL;

// Notify to Java
int Notify(int status)
{
    JNIEnv *env = NULL;
    int isAttached = 0; 

    if(gvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        //__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "jvm getenv failed use AttachCurrentThread \n ");
        if(gvm->AttachCurrentThread(&env, NULL) != JNI_OK)
        {
            //__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "jvm AttachCurrentThread failed \n ");
            return -1;
        }
        isAttached = 1;
    }
    
    if(!notify_cb || !mClass)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "updateState can not found ");
        goto END;
    }
 
    if(status == MEDIA_PLAYBACK_COMPLETE)
    {
        delete dtPlayer;
        dtPlayer = NULL;
    }
    
    env->CallStaticVoidMethod(mClass,notify_cb,status);

END:
    if(isAttached)
        gvm->DetachCurrentThread();  
    return 0;
}

dtpListenner::dtpListenner(JNIEnv *env, jobject thiz, jobject weak_thiz)
{
    jclass clazz = env->GetObjectClass(thiz);
    if(clazz == NULL)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "can not find DtPlayer \n ");
        return;
    }
    mClass = (jclass)env->NewGlobalRef(clazz);
    mObject = env->NewGlobalRef(weak_thiz);
}

dtpListenner::~dtpListenner()
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);
}

int dtpListenner::notify(int msg, int ext1, int ext2)
{
#if 0
    JNIEnv *env = NULL;
    int isAttached = 0; 
    if(gvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "jvm getenv failed use AttachCurrentThread \n ");
        if(gvm->AttachCurrentThread(&env, NULL) != JNI_OK)
        {
            __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "jvm AttachCurrentThread failed \n ");
            return -1;
        }
        isAttached = 1;
    }
    
    if(!fields.post_event || !mClass)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "updateState can not found ");
        goto END;
    }
    
    env->CallStaticVoidMethod(mClass, fields.post_event, msg, ext1, ext2);
    //env->CallVoidMethod(mObj,notify_cb,status);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "NOtify with listener \n ");
END:
    if(isAttached)
        gvm->DetachCurrentThread();  
#endif

#if 1
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter notify \n ");
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, NULL);

    if(env->ExceptionCheck())
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "An Exception occured while notifying an event \n ");
        env->ExceptionClear();
    }
#endif
    return 0;
}

static DTPlayer * setMediaPlayer(JNIEnv *env, jobject thiz, DTPlayer *player)
{
    dt_lock(&sLock);
    DTPlayer *old = (DTPlayer *)env->GetIntField(thiz, fields.context);
    env->SetIntField(thiz, fields.context, (int)player);
    dt_unlock(&sLock);
    return old;
}

static DTPlayer *getMediaPlayer(JNIEnv *env, jobject thiz)
{
    dt_lock(&sLock);
    DTPlayer *dtp = (DTPlayer *)env->GetIntField(thiz, fields.context);
    dt_unlock(&sLock);
    return dtp;
}

//================================================

static void android_dtplayer_native_init(JNIEnv *env)
{
    jclass clazz;

    clazz = env->FindClass(kClassName);
    if (clazz == NULL) {
        return;
    }

    fields.context = env->GetFieldID(clazz, "mNativeContext", "I");
    if (fields.context == NULL) {
        return;
    }

    fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative",
                                               "(Ljava/lang/Object;IIILjava/lang/Object;)V");
    if (fields.post_event == NULL) {
        return;
    }

#if 0
    fields.surface_texture = env->GetFieldID(clazz, "mNativeSurfaceTexture", "I");
    if (fields.surface_texture == NULL) {
        return;
    }
#endif
}

static int android_dtplayer_native_setup(JNIEnv *env, jobject obj, jobject weak_thiz)
{
    DTPlayer *mp = new DTPlayer();
    if(mp == NULL)
    {
        //jniThrowExcption(env, "java/lang/RuntimeException", "Out of memory");
        return -1;
    }
    dtpListenner *listenner = new dtpListenner(env, obj, weak_thiz);
    mp->setListenner(listenner);
    setMediaPlayer(env, obj, mp);
    return 0;
}

static int android_dtplayer_native_release(JNIEnv *env, jobject obj)
{
    DTPlayer *mp = setMediaPlayer(env, obj, 0);
    if(mp)
    {
        delete mp;
    }
    return 0;
}

int android_dtplayer_native_setDataSource(JNIEnv *env, jobject thiz, jstring url)
{
    int ret = 0;
    jboolean isCopy;
    const char * file_name = env->GetStringUTFChars(url, &isCopy);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter setDataSource, path: [%s] size:%d ",file_name,strlen(file_name));

    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "setDataSource, failed, mp == null ");
        return -1;
    }
    
    ret = mp->setDataSource(file_name);
    if(ret < 0)
    {
        delete dtPlayer;
        dtPlayer = NULL;
    }
    return ret;
}

int android_dtplayer_native_prePare(JNIEnv *env, jobject thiz)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter prePare");
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;

    mp->prePare();
    return 0;
}

int android_dtplayer_native_prepareAsync(JNIEnv *env, jobject obj)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter prePareAsync");
    if(!dtPlayer)
        return -1;
    dtPlayer->prePareAsync();
    return 0;
}

int android_dtplayer_native_start(JNIEnv *env, jobject obj)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter start");
    if(!dtPlayer)
        return -1;
    return dtPlayer->start();
}

int android_dtplayer_native_pause(JNIEnv *env, jobject obj)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter pause");
    if(!dtPlayer)
        return -1;
    return dtPlayer->pause();
}

int android_dtplayer_native_seekTo(JNIEnv *env, jobject obj, jint pos)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter seekTo pos:%d s",pos);
    if(!dtPlayer)
        return -1;
    return dtPlayer->seekTo(pos);
}

int android_dtplayer_native_stop(JNIEnv *env, jobject obj)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter stop ");
    int ret = 0;
    if(!dtPlayer)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "player already destroyed, quit ");
        return -1;
    }
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "native stop enter \n ");
    ret = dtPlayer->stop();
    {
        if(ret == -1)
            return -1;
    }
    while(dtPlayer->isQuitOK() == 0)
    {
        usleep(10000);
    }

    delete dtPlayer;
    dtPlayer = NULL;
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "native stop exit \n ");
    return 0;
}

int android_dtplayer_native_reset(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->reset();
}

void android_dtplayer_native_releaseSurface(JNIEnv *env, jobject obj)
{

}

int android_dtplayer_native_setVideoSize(JNIEnv *env, jobject obj, int w, int h)
{
    if(!dtPlayer)
        return -1;
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "setVideoSize, w:%d h:%d \n ",w,h);
    dtPlayer->setVideoSize(w, h);
}

int android_dtplayer_native_setVideoMode(JNIEnv *env, jobject obj, int mode)
{
    if(!dtPlayer)
        return -1;
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "setMode, mode:%d \n ", mode);
    return dtPlayer->setVideoMode(mode);
}

int android_dtplayer_native_getVideoWidth(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->getVideoWidth();
}

int android_dtplayer_native_getVideoHeight(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->getVideoHeight();
}

int android_dtplayer_native_isPlaying(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->isPlaying();
}

int android_dtplayer_native_getCurrentPosition(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return 0;
    return dtPlayer->getCurrentPosition();
}

int android_dtplayer_native_getDuration(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return 0;
    return dtPlayer->getDuration();
}


//-----------------------------------------------------
//OPENGL -- common

static void check_gl_error(const char* op)
{
	GLint error;
	for (error = glGetError(); error; error = glGetError())
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,"after %s() glError (0x%x)\n", op, error);
}

static void checkGlError(const char* op)
{
	GLint error;
	for (error = glGetError(); error; error = glGetError())
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,"after %s() glError (0x%x)\n", op, error);
}
#ifdef USE_OPENGL_V2

static const char* FRAG_SHADER = 
    "varying lowp vec2 tc;\n"
    "uniform sampler2D SamplerY;\n"
    "uniform sampler2D SamplerU;\n"
    "uniform sampler2D SamplerV;\n"
    "void main(void)\n"
    "{\n"
        "mediump vec3 yuv;\n"
        "lowp vec3 rgb;\n"
        "yuv.x = texture2D(SamplerY, tc).r;\n"
        "yuv.y = texture2D(SamplerU, tc).r - 0.5;\n"
        "yuv.z = texture2D(SamplerV, tc).r - 0.5;\n"
        "rgb = mat3( 1,   1,   1,\n"
             "0,       -0.39465,  2.03211,\n"
             "1.13983,   -0.58060,  0) * yuv;\n"
        "gl_FragColor = vec4(rgb, 1);\n"
    "}\n";

static const char* VERTEX_SHADER =
    "attribute vec4 vPosition;    \n"
    "attribute vec2 a_texCoord;   \n"
    "varying vec2 tc;     \n"
    "void main()                  \n"
    "{                            \n" 
    "   gl_Position = vPosition;  \n"
    "   tc = a_texCoord;  \n" 
    "}                            \n";


static GLuint gles2_bindTexture(GLuint texture, const uint8_t *buffer, GLuint w , GLuint h)
{
    checkGlError("glGenTextures"); 
    glBindTexture ( GL_TEXTURE_2D, texture );  
    checkGlError("glBindTexture");  
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);  
    checkGlError("glTexImage2D");  
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );  
    checkGlError("glTexParameteri");  
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );  
    checkGlError("glTexParameteri");  
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );  
    checkGlError("glTexParameteri");  
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );  
    checkGlError("glTexParameteri");

    return texture; 
}

static void gles2_renderFrame() 
{
    // Galaxy Nexus 4.2.2
    static GLfloat squareVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f,
    };

    static GLfloat coordVertices[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f,  0.0f,
        1.0f,  0.0f,
    };
    
    // HUAWEIG510-0010 4.1.1 
    static GLfloat squareVertices1[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f,  1.0f,
        1.0f,  1.0f,
    };
    static GLfloat coordVertices1[] = {
        -1.0f, 1.0f,
        1.0f, 1.0f,
        -1.0f,  -1.0f,
        1.0f,  -1.0f,
    };

    glClearColor(0.5f, 0.5f, 0.5f, 1);
    checkGlError("glClearColor");
    
    glClear(GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    GLint tex_y = glGetUniformLocation(gl_ctx.simpleProgram, "SamplerY");
    checkGlError("glGetUniformLocation");
    GLint tex_u = glGetUniformLocation(gl_ctx.simpleProgram, "SamplerU");
    checkGlError("glGetUniformLocation");
    GLint tex_v = glGetUniformLocation(gl_ctx.simpleProgram, "SamplerV");
    checkGlError("glGetUniformLocation");

    glBindAttribLocation(gl_ctx.simpleProgram, ATTRIB_VERTEX, "vPosition");
    checkGlError("glBindAttribLocation");
    glBindAttribLocation(gl_ctx.simpleProgram, ATTRIB_TEXTURE, "a_texCoord");
    checkGlError("glBindAttribLocation");

    if(gl_ctx.vertex_index == 0)
        glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices); 
    else
        glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices1); 
        
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    checkGlError("glEnableVertexAttribArray");
    
    if(gl_ctx.vertex_index == 0)
        glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, coordVertices);
    else
        glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, coordVertices1);
    
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(ATTRIB_TEXTURE);
    checkGlError("glEnableVertexAttribArray");

    glActiveTexture(GL_TEXTURE0);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, gl_ctx.g_texYId);
    checkGlError("glBindTexture");
    glUniform1i(tex_y, 0);
    checkGlError("glUniform1i");

    glActiveTexture(GL_TEXTURE1);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, gl_ctx.g_texUId);
    checkGlError("glBindTexture");
    glUniform1i(tex_u, 1);
    checkGlError("glUniform1i");

    glActiveTexture(GL_TEXTURE2);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, gl_ctx.g_texVId);
    checkGlError("glBindTexture");
    glUniform1i(tex_v, 2);
    checkGlError("glUniform1i");

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    checkGlError("glDrawArrays");
}


static GLuint buildShader(const char* source, GLenum shaderType)   
{  
    GLuint shaderHandle = glCreateShader(shaderType);  
  
    if (shaderHandle)  
    {  
        glShaderSource(shaderHandle, 1, &source, 0);  
        glCompileShader(shaderHandle);  
  
        GLint compiled = 0;  
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compiled);  
        if (!compiled)  
        {  
            GLint infoLen = 0;  
            glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);  
            if (infoLen)  
            {  
                char* buf = (char*) malloc(infoLen);  
                if (buf)  
                {  
                    glGetShaderInfoLog(shaderHandle, infoLen, NULL, buf);  
		            __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG," error: Could not compile shader %d \n %s\n", shaderType, buf);
                    free(buf);  
                }  
                glDeleteShader(shaderHandle);  
                shaderHandle = 0;  
            }  
        }  
  
    }  
      
    return shaderHandle;  
}

static GLuint buildProgram(const char* vertexShaderSource,  
        const char* fragmentShaderSource)   
{  
    GLuint vertexShader = buildShader(vertexShaderSource, GL_VERTEX_SHADER);  
    GLuint fragmentShader = buildShader(fragmentShaderSource, GL_FRAGMENT_SHADER);  
    GLuint programHandle = glCreateProgram();  
  
    if (programHandle)  
    {  
        glAttachShader(programHandle, vertexShader);  
        checkGlError("glAttachShader");  
        glAttachShader(programHandle, fragmentShader);  
        checkGlError("glAttachShader");  
        glLinkProgram(programHandle);  
  
        GLint linkStatus = GL_FALSE;  
        glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);  
        if (linkStatus != GL_TRUE) {  
            GLint bufLength = 0;  
            glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &bufLength);  
            if (bufLength) {  
                char* buf = (char*) malloc(bufLength);  
                if (buf) {  
                    glGetProgramInfoLog(programHandle, bufLength, NULL, buf);  
		            __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG," error: Could not link programe: %s\n", buf);
                    free(buf);  
                }  
            }  
            glDeleteProgram(programHandle);  
            programHandle = 0;  
        }  
  
    }  
  
    return programHandle;  
}

static void gles2_init()   
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    checkGlError("glClearColor");

    memset(&gl_ctx,0,sizeof(gles2_ctx_t));
    gl_ctx.simpleProgram = buildProgram(VERTEX_SHADER, FRAG_SHADER);  
    glUseProgram(gl_ctx.simpleProgram); 
    glGenTextures(1, &gl_ctx.g_texYId);  
    glGenTextures(1, &gl_ctx.g_texUId);  
    glGenTextures(1, &gl_ctx.g_texVId); 
    checkGlError("glGenTextures");


    char *glExtension = (char *)glGetString(GL_EXTENSIONS);
    if(strstr(glExtension, "GL_AMD_compressed_ATC_texture") != NULL)
        gl_ctx.vertex_index = 1; 
    else
        gl_ctx.vertex_index = 0;

    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "opengl esv2 init ok, ext:%s \n ", glExtension);
}

static void gles2_uninit()  
{  
    gl_ctx.g_width = 0;  
    gl_ctx.g_height = 0;  
    if (gl_ctx.frame.data[0])  
    {  
        free(gl_ctx.frame.data[0]);  
        gl_ctx.frame.data[0] = NULL;  
    }
}

static int gles2_surface_changed(int w, int h)
{
    gl_ctx.g_width = w;
	gl_ctx.g_height = h;
    gl_ctx.status = GLRENDER_STATUS_RUNNING;
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    checkGlError("glClearColor");
    
    int width = gl_ctx.g_width;
    int height = gl_ctx.g_height;

    glViewport(0, 0, width, height);

    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "on surface changed\n ");
}

static int gles2_draw_frame()
{
    if(gl_ctx.status == GLRENDER_STATUS_IDLE)
        return 0;
    
    if(gl_ctx.invalid_frame == 0)
    {
        //__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "update_frame, No frame to draw \n");
        return 0;
    }

    if(!gl_ctx.frame.data[0])
        return 0;


    //Fixme: scale to dst width
    
    int width = gl_ctx.g_width;
    int height = gl_ctx.g_height;
    dt_av_frame_t *frame = &gl_ctx.frame;
    uint8_t *data = frame->data[0];
    //glViewport(0, 0, width, height);  
    gles2_bindTexture(gl_ctx.g_texYId, data, width, height);  
    gles2_bindTexture(gl_ctx.g_texUId, data + width * height, width/2, height/2);  
    gles2_bindTexture(gl_ctx.g_texVId, data + width * height * 5 / 4, width/2, height/2);  

    gles2_renderFrame();
    if(frame->data[0])
        free(frame->data[0]);
    frame->data[0] = NULL;
    gl_ctx.invalid_frame = 0;
    return 0;
}

#endif

int android_dtplayer_native_onSurfaceCreated(JNIEnv *env, jobject obj)
{
#ifdef USE_OPENGL_V2
    gles2_init();
#endif
    dt_lock_init (&gl_ctx.mutex, NULL);
    return 0;
}

int android_dtplayer_native_onSurfaceChanged(JNIEnv *env, jobject obj, int w, int h)
{
    dt_lock(&gl_ctx.mutex);
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "onSurfaceChanged, w:%d h:%d \n",w,h);

#ifdef USE_OPENGL_V2
    gles2_surface_changed(w, h);
#endif

    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "onSurfaceChanged ok\n");
    
    dt_unlock(&gl_ctx.mutex);
    android_dtplayer_native_setVideoSize(env, obj, w, h);
    return 0;
}

extern "C" int update_frame(dt_av_frame_t *frame)
{
    int ret = 0;
    dt_lock (&gl_ctx.mutex);
    dt_av_frame_t *orig_frame = &gl_ctx.frame;
    // check frame not displayed
    if(orig_frame->data[0])
    {
        free(orig_frame->data[0]);
    }
    memcpy(&gl_ctx.frame, frame, sizeof(dt_av_frame_t));
    gl_ctx.invalid_frame = 1;
    dt_unlock (&gl_ctx.mutex);
    Notify(MEDIA_FRESH_VIDEO); // update view
}

int android_dtplayer_native_onDrawFrame(JNIEnv *env, jobject obj)
{
    dt_lock(&gl_ctx.mutex);

#ifdef USE_OPENGL_V2
    gles2_draw_frame();
#endif

    dt_unlock(&gl_ctx.mutex);
    return 0;
}

//-----------------------------------------------------

//-----------------------------------------------------
// Audio Effect Interface
static int android_dtplayer_native_setAudioEffect(int effect_id)
{
#ifdef ENABLE_DTAP
    dtap_change_effect(effect_id);
#endif
    return 0;
}

//-----------------------------------------------------

static JNINativeMethod g_Methods[] = {
    //New API
    {"native_init",               "()V",                      (void*) android_dtplayer_native_init},
    {"native_setup",              "(Ljava/lang/Object;)I",    (void*) android_dtplayer_native_setup},
    {"native_release",            "()I",                      (void*) android_dtplayer_native_release},
    {"native_setDataSource",      "(Ljava/lang/String;)I",    (void*) android_dtplayer_native_setDataSource},
    {"native_prePare",            "()I",                      (void*) android_dtplayer_native_prePare},
    {"native_prePareAsync",       "()I",                      (void*) android_dtplayer_native_prepareAsync},
    {"native_start",              "()I",                      (void*) android_dtplayer_native_start},
    {"native_pause",              "()I",                      (void*) android_dtplayer_native_pause},
    {"native_seekTo",             "(I)I",                     (void*) android_dtplayer_native_seekTo},
    {"native_stop",               "()I",                      (void*) android_dtplayer_native_stop},
    {"native_reset",              "()I",                      (void*) android_dtplayer_native_reset},
    {"native_setVideoMode",       "(I)I",                     (void*) android_dtplayer_native_setVideoMode},
    {"native_setVideoSize",       "(II)I",                    (void*) android_dtplayer_native_setVideoSize},
    {"native_getVideoWidth",      "()I",                      (void*) android_dtplayer_native_getVideoWidth},
    {"native_getVideoHeight",     "()I",                      (void*) android_dtplayer_native_getVideoHeight},
    {"native_isPlaying",          "()I",                      (void*) android_dtplayer_native_isPlaying},
    {"native_getCurrentPosition", "()I",                      (void*) android_dtplayer_native_getCurrentPosition},
    {"native_getDuration",        "()I",                      (void*) android_dtplayer_native_getDuration},
    
    {"native_onSurfaceCreated",   "()I",                      (void*) android_dtplayer_native_onSurfaceCreated},
    {"native_onSurfaceChanged",   "(II)I",                    (void*) android_dtplayer_native_onSurfaceChanged},
    {"native_onDrawFrame",        "()I",                      (void*) android_dtplayer_native_onDrawFrame},
    
    {"native_setAudioEffect",     "(I)I",                     (void*) android_dtplayer_native_setAudioEffect},
};

static int register_android_dtplayer(JNIEnv *env)
{
#if 0
    jclass clazz;
    clazz = env->FindClass(kClassName);
	if (clazz == NULL) {
		fprintf(stderr, "Native registration unable to find class '%s'\n",kClassName);
		return JNI_FALSE;
	}
	if (env->RegisterNatives(clazz, g_Methods, NELEM(g_Methods)) < 0) {
		fprintf(stderr, "RegisterNatives failed for '%s'\n", kClassName);
		return JNI_FALSE;
	}

    return JNI_TRUE;
#else
    return AndroidRuntime::registerNativeMethods(env, kClassName, g_Methods, NELEM(g_Methods));
#endif

}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;
    gvm = vm;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        //ALOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (register_android_dtplayer(env) < 0) {
        //ALOGE("ERROR: MediaPlayer native registration failed\n");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;
    dt_lock_init(&sLock, NULL);
bail:
    return result;
}


