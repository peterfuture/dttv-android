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


#ifdef USE_OPENGL_V1

#include <GLES/gl.h>
#include <GLES/glext.h>

#endif

#include "dt_lock.h"

//#define USE_LISTENNER 0

//-----------------------------------------------------------

#define RGB565(r, g, b)  (((r) << (5+6)) | ((g) << 6) | (b))
#define GLRENDER_STATUS_IDLE 0
#define GLRENDER_STATUS_RUNNING 1
#define GLRENDER_STATUS_QUIT 2

#ifdef USE_OPENGL_V1

static GLuint s_disable_caps[] = {
    GL_FOG,
	GL_LIGHTING,
	GL_CULL_FACE,
	GL_ALPHA_TEST,
	GL_BLEND,
	GL_COLOR_LOGIC_OP,
	GL_DITHER,
	GL_STENCIL_TEST,
	GL_DEPTH_TEST,
	GL_COLOR_MATERIAL,
	0
};
typedef struct{
	uint16_t *frame;
    int next_canvas;
    int frame_size;
	int width;
	int height;
	int status;
    int invalid_frame;
    GLuint s_texture;

    dt_lock_t mutex;
}gl_ctx_t;
static gl_ctx_t gl_ctx;

#endif

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
    uint8_t *g_buffer;
    int buffer_size;
	int status;
    int invalid_frame;
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
//================================================

static JavaVM *gvm = NULL;
static jclass mClass = NULL;
static jmethodID notify_cb = NULL;

// Notify to Java
int Notify(int status)
{
#ifndef USE_LISTENNER 
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
    else
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "jvm getenv ok \n ");
    
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
#endif
    return 0;
}

dtpListenner::dtpListenner(JNIEnv *env, jobject thiz)
{
    jclass clazz = env->GetObjectClass(thiz);
    mClass = (jclass)env->NewGlobalRef(clazz);
    mObj = env->NewGlobalRef(thiz);
    notify_cb = env->GetStaticMethodID(mClass, "updateState", "(I)V");
}

dtpListenner::~dtpListenner()
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->DeleteGlobalRef(mObj);
    env->DeleteGlobalRef(mClass);
}

int dtpListenner::notify(int status)
{
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
    
    if(!notify_cb || !mClass)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "updateState can not found ");
        goto END;
    }
    
    env->CallStaticVoidMethod(mClass,notify_cb,status);
    //env->CallVoidMethod(mObj,notify_cb,status);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "NOtify with listener \n ");
END:
    if(isAttached)
        gvm->DetachCurrentThread();  

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

static int dtp_setup(JNIEnv *env, jobject obj)
{
#ifdef USE_LISTENNER
    DTPlayer *mp = new DTPlayer();
    if(!mp)
    {
        return -1;
    }
    dtpListenner *listenner = new dtpListenner(env, obj);
    mp->setListenner(listenner);
    setMediaPlayer(env, obj, mp);
    return 0;
#endif
}

static int dtp_release(JNIEnv *env, jobject obj)
{
#ifdef USE_LISTENNER
    DTPlayer *mp = setMediaPlayer(env, obj, 0);
    if(mp)
    {
        delete mp;
    }
    return 0;
#endif
}

int dtp_setDataSource(JNIEnv *env, jobject obj, jstring url)
{
    int ret = 0;
    jboolean isCopy;
    const char * file_name = env->GetStringUTFChars(url, &isCopy);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter setDataSource, path: [%s] size:%d ",file_name,strlen(file_name));

    if(!dtPlayer)
    {
        dtPlayer = new DTPlayer;
        dtpListenner *listenner = new dtpListenner(env, obj);
        dtPlayer->setListenner(listenner);
    }
    else
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "setDataSource, failed, another player is running ");
        return -1;
    }

#ifndef USE_LISTENNER
    jclass clazz = env->GetObjectClass(obj);
    mClass = (jclass)env->NewGlobalRef(clazz);
    notify_cb = env->GetStaticMethodID(mClass, "updateState", "(I)V");
#endif
    ret = dtPlayer->setDataSource(file_name);
    if(ret < 0)
    {
        delete dtPlayer;
        dtPlayer = NULL;
    }
    return ret;
}

int dtp_prePare(JNIEnv *env, jobject obj)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter prePare");
    if(!dtPlayer)
        return -1;
    dtPlayer->prePare();
    return 0;
}

int dtp_prepareAsync(JNIEnv *env, jobject obj)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter prePareAsync");
    if(!dtPlayer)
        return -1;
    dtPlayer->prePareAsync();
    return 0;
}

int dtp_start(JNIEnv *env, jobject obj)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter start");
    if(!dtPlayer)
        return -1;
    return dtPlayer->start();
}

int dtp_pause(JNIEnv *env, jobject obj)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter pause");
    if(!dtPlayer)
        return -1;
    return dtPlayer->pause();
}

int dtp_seekTo(JNIEnv *env, jobject obj, jint pos)
{
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Enter seekTo pos:%d s",pos);
    if(!dtPlayer)
        return -1;
    return dtPlayer->seekTo(pos);
}

int dtp_stop(JNIEnv *env, jobject obj)
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

int dtp_reset(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->reset();
}

void dtp_releaseSurface(JNIEnv *env, jobject obj)
{

}

int dtp_setVideoSize(JNIEnv *env, jobject obj, int w, int h)
{
    if(!dtPlayer)
        return -1;
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "setVideoSize, w:%d h:%d \n ",w,h);
    dtPlayer->setVideoSize(w, h);
}

int dtp_getVideoWidth(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->getVideoWidth();
}

int dtp_getVideoHeight(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->getVideoHeight();
}

int dtp_isPlaying(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->isPlaying();
}

int dtp_getCurrentPosition(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return 0;
    return dtPlayer->getCurrentPosition();
}

int dtp_getDuration(JNIEnv *env, jobject obj)
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
#if 0
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
#else
    // HUAWEIG510-0010 4.1.1 
    static GLfloat squareVertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f,  1.0f,
        1.0f,  1.0f,
    };
    static GLfloat coordVertices[] = {
        -1.0f, 1.0f,
        1.0f, 1.0f,
        -1.0f,  -1.0f,
        1.0f,  -1.0f,
    };
#endif

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

    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices); 
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    checkGlError("glEnableVertexAttribArray");

    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, coordVertices);
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
    gl_ctx.g_buffer = NULL;  
    gl_ctx.simpleProgram = buildProgram(VERTEX_SHADER, FRAG_SHADER);  
    glUseProgram(gl_ctx.simpleProgram); 
    glGenTextures(1, &gl_ctx.g_texYId);  
    glGenTextures(1, &gl_ctx.g_texUId);  
    glGenTextures(1, &gl_ctx.g_texVId); 
    checkGlError("glGenTextures");

    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "opengl esv2 init ok\n ");
}

static void gles2_uninit()  
{  
    gl_ctx.g_width = 0;  
    gl_ctx.g_height = 0;  
    if (gl_ctx.g_buffer)  
    {  
        free(gl_ctx.g_buffer);  
        gl_ctx.g_buffer = NULL;  
    }  
}

static int gles2_surface_changed(int w, int h)
{
    gl_ctx.g_width = w;
	gl_ctx.g_height = h;
	gl_ctx.buffer_size = w*h + w*h/2;
    if(gl_ctx.g_buffer)
        free(gl_ctx.g_buffer);

    gl_ctx.g_buffer = NULL;
    gl_ctx.status = GLRENDER_STATUS_RUNNING;
 
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    checkGlError("glClearColor");
    
    int width = gl_ctx.g_width;
    int height = gl_ctx.g_height;

    glViewport(0, 0, width, height);

    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "on surface changed\n ");
}

static int gles2_update_frame(uint8_t *buffer, int size)
{
    if(gl_ctx.g_buffer)
        free(gl_ctx.g_buffer);

    gl_ctx.g_buffer = buffer;

    gl_ctx.invalid_frame = 1;
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "update frame v2\n ");
    return 0;
}

static int gles2_draw_frame()
{
    if(gl_ctx.status == GLRENDER_STATUS_IDLE)
        return 0;
    
    if(gl_ctx.invalid_frame == 0)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "update_frame, No frame to draw \n");
        return 0;
    }

    if(!gl_ctx.g_buffer)
        return 0;

    int width = gl_ctx.g_width;
    int height = gl_ctx.g_height;

    //glViewport(0, 0, width, height);  
    gles2_bindTexture(gl_ctx.g_texYId, gl_ctx.g_buffer, width, height);  
    gles2_bindTexture(gl_ctx.g_texUId, gl_ctx.g_buffer + width * height, width/2, height/2);  
    gles2_bindTexture(gl_ctx.g_texVId, gl_ctx.g_buffer + width * height * 5 / 4, width/2, height/2);  

    gles2_renderFrame();
    if(gl_ctx.g_buffer)
        free(gl_ctx.g_buffer);
    gl_ctx.g_buffer = NULL;
    gl_ctx.invalid_frame = 0;
    return 0;
}

#endif


#ifdef USE_OPENGL_V1

static void gles1_init()
{
    memset(&gl_ctx,0,sizeof(gl_ctx_t));
}

static int gles1_surface_changed(int w, int h)
{
    gl_ctx.width = w;
	gl_ctx.height = h;
	gl_ctx.frame_size = w*h*2;
    if(gl_ctx.frame)
        free(gl_ctx.frame);

    gl_ctx.status = GLRENDER_STATUS_RUNNING;

	glDeleteTextures(1, &gl_ctx.s_texture);
	GLuint *start = s_disable_caps;
	while (*start)
		glDisable(*start++);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &gl_ctx.s_texture);
	glBindTexture(GL_TEXTURE_2D, gl_ctx.s_texture);
	glTexParameterf(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glShadeModel(GL_FLAT);
	check_gl_error("glShadeModel");
	glColor4x(0x10000, 0x10000, 0x10000, 0x10000);
	check_gl_error("glColor4x");
	//int rect[4] = {0, TEXTURE_HEIGHT, TEXTURE_WIDTH, -TEXTURE_HEIGHT};
	int rect[4] = {0, h, w, -h};
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);
	check_gl_error("glTexParameteriv");

    gl_ctx.status = GLRENDER_STATUS_RUNNING;

    //dtPlayer->setVideoSize(w,h);//do not resize here

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    return 0;
}

static int gles1_update_frame(uint8_t *buf,int size)
{
    int cp_size = size;
    if(size > gl_ctx.frame_size)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "update_frame,in size:%d larger than out size:%d  \n",gl_ctx.frame_size,gl_ctx.frame_size);
        return -1;
    }
    
    if(gl_ctx.frame) // too slow
        free(gl_ctx.frame);
    
    cp_size = (size < gl_ctx.frame_size)?size:gl_ctx.frame_size;
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "update_frame, cpsize:%d size:%d bufsize:%d \n",cp_size,size,gl_ctx.frame_size);
    
    gl_ctx.frame = (uint16_t *)buf;

    gl_ctx.invalid_frame = 1;
    return 0;
}

static int gles1_draw_frame()
{
    if(gl_ctx.status == GLRENDER_STATUS_IDLE)
        return 0;
    
    if(gl_ctx.invalid_frame == 0)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "update_frame, No frame to draw \n");
        return 0;
    }

    if(!gl_ctx.frame)
        return 0;
    //update_pixel_test();
    glClear(GL_COLOR_BUFFER_BIT);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "clear buffer first2\n");
    
    glTexImage2D(GL_TEXTURE_2D,		/* target */
			0,			/* level */
			GL_RGB,			/* internal format */
			gl_ctx.width,		/* width */
			gl_ctx.height,		/* height */
			0,			/* border */
			GL_RGB,			/* format */
			GL_UNSIGNED_SHORT_5_6_5,/* type */
			gl_ctx.frame);		/* pixels */
    gl_ctx.next_canvas = 0;

	glDrawTexiOES(0, 0, 0, gl_ctx.width, gl_ctx.height);
    gl_ctx.invalid_frame = 0;
    if(gl_ctx.frame)
        free(gl_ctx.frame);
    gl_ctx.frame = NULL;
    return 0;
}

#endif



int dtp_onSurfaceCreated(JNIEnv *env, jobject obj)
{
#ifdef USE_OPENGL_V1
    gles1_init();
#endif
#ifdef USE_OPENGL_V2
    gles2_init();
#endif
    dt_lock_init (&gl_ctx.mutex, NULL);
    return 0;
}

int dtp_onSurfaceChanged(JNIEnv *env, jobject obj, int w, int h)
{
    dt_lock(&gl_ctx.mutex);
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "onSurfaceChanged, w:%d h:%d \n",w,h);

#ifdef USE_OPENGL_V1
    gles1_surface_changed(w, h);
#endif

#ifdef USE_OPENGL_V2
    gles2_surface_changed(w, h);
#endif

    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "onSurfaceChanged ok\n");
END:
    dt_unlock(&gl_ctx.mutex);
    return 0;
}

extern "C" int update_frame(uint8_t *buf,int size)
{
    int ret = 0;
    dt_lock (&gl_ctx.mutex);

#ifdef USE_OPENGL_V1
    ret = gles1_update_frame(buf, size);
#endif

#ifdef USE_OPENGL_V2
    ret = gles2_update_frame(buf, size);
#endif
    
    dt_unlock (&gl_ctx.mutex);
    
    if(ret < 0)
        return ret;

    Notify(MEDIA_FRESH_VIDEO); // update view
}

int dtp_onDrawFrame(JNIEnv *env, jobject obj)
{
    dt_lock(&gl_ctx.mutex);

#ifdef USE_OPENGL_V1
    gles1_draw_frame();
#endif

#ifdef USE_OPENGL_V2
    gles2_draw_frame();
#endif

    dt_unlock(&gl_ctx.mutex);
    return 0;
}

//-----------------------------------------------------

//-----------------------------------------------------
// Audio Effect Interface
static int dtp_setAudioEffect(int effect_id)
{
    return 0;
}

//-----------------------------------------------------

static JNINativeMethod g_Methods[] = {
    //New API
    {"native_setup",              "()I",                      (void*) dtp_setup},
    {"native_release",            "()I",                      (void*) dtp_release},
    {"native_setDataSource",      "(Ljava/lang/String;)I",    (void*) dtp_setDataSource},
    {"native_prePare",            "()I",                      (void*) dtp_prePare},
    {"native_prePareAsync",       "()I",                      (void*) dtp_prepareAsync},
    {"native_start",              "()I",                      (void*) dtp_start},
    {"native_pause",              "()I",                      (void*) dtp_pause},
    {"native_seekTo",             "(I)I",                     (void*) dtp_seekTo},
    {"native_stop",               "()I",                      (void*) dtp_stop},
    {"native_reset",              "()I",                      (void*) dtp_reset},
    {"native_setVideoSize",       "(II)I",                    (void*) dtp_setVideoSize},
    {"native_getVideoWidth",      "()I",                      (void*) dtp_getVideoWidth},
    {"native_getVideoHeight",     "()I",                      (void*) dtp_getVideoHeight},
    {"native_isPlaying",          "()I",                      (void*) dtp_isPlaying},
    {"native_getCurrentPosition", "()I",                      (void*) dtp_getCurrentPosition},
    {"native_getDuration",        "()I",                      (void*) dtp_getDuration},
    
    {"native_onSurfaceCreated",   "()I",                      (void*) dtp_onSurfaceCreated},
    {"native_onSurfaceChanged",   "(II)I",                    (void*) dtp_onSurfaceChanged},
    {"native_onDrawFrame",        "()I",                      (void*) dtp_onDrawFrame},
    
    {"native_setAudioEffect",     "(I)I",                      (void*) dtp_setAudioEffect},
};

static int register_android_dtplayer(JNIEnv *env)
{
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


