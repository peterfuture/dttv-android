// ------------------------------------------------------------
//OPENGL ESV2

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <jni.h>
#include <android/log.h>

#include "dt_av.h"
#include "dt_lock.h"
#include "android_opengl.h"
#include "android_dtplayer.h"

#define LOG_TAG "OPENGL-ANDROID"

namespace android{

static gles2_ctx_t gl_ctx;
static dt_lock_t mutex;

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

extern "C" int update_frame(dt_av_frame_t *frame)
{
    int ret = 0;
    dt_lock (&mutex);

    if(gl_ctx.initialized == 0)
    {
        free(frame->data[0]);
        frame->data[0] = NULL;
        __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "gles2 context not iniited yet \n");
        dt_unlock (&mutex);
        return 0;
    }

    dt_av_frame_t *orig_frame = &gl_ctx.frame;
    // check frame not displayed
    if(orig_frame->data[0])
    {
        free(orig_frame->data[0]);
    }
    memcpy(&gl_ctx.frame, frame, sizeof(dt_av_frame_t));
    gl_ctx.invalid_frame = 1;
    dt_unlock (&mutex);

    DTPlayer *mp = gl_ctx.mp;
    if(!mp)
    {
        __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "mp null \n");
        return 0;
    }
    mp->Notify(MEDIA_FRESH_VIDEO);

    return 0;
}

void gles2_setup()
{
    memset(&gl_ctx,0,sizeof(gles2_ctx_t));
    dt_lock_init(&mutex, NULL);
}

void gles2_reg_player(DTPlayer *mp)
{
    dt_lock (&mutex);
    gl_ctx.mp = mp;
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "[%s:%d] register ok %p\n", __FUNCTION__, __LINE__, mp);
    dt_unlock (&mutex);
}

void gles2_init() 
{
    dt_lock (&mutex);
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
    gl_ctx.initialized = 1;
    dt_unlock (&mutex);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "opengl esv2 init ok, ext:%s\n ", glExtension);
}

void gles2_release() 
{  
    dt_lock(&mutex);

    gl_ctx.g_width = 0;  
    gl_ctx.g_height = 0;  
    if (gl_ctx.frame.data[0])  
    {  
        free(gl_ctx.frame.data[0]);  
        gl_ctx.frame.data[0] = NULL;  
    }
    gl_ctx.mp = NULL;
    gl_ctx.initialized = 0;
    dt_unlock(&mutex);
    memset(&gl_ctx,0,sizeof(gles2_ctx_t));
}

int gles2_surface_changed(int w, int h)
{
    dt_lock(&mutex);

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
    dt_unlock(&mutex);
}

int gles2_draw_frame()
{
    dt_lock(&mutex);
    int width = gl_ctx.g_width;
    int height = gl_ctx.g_height;
    dt_av_frame_t *frame = &gl_ctx.frame;
    uint8_t *data = NULL;


    if(gl_ctx.status == GLRENDER_STATUS_IDLE)
        goto END;
    
    if(gl_ctx.invalid_frame == 0)
    {
        //__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "update_frame, No frame to draw \n");
        goto END;
    }

    if(!gl_ctx.frame.data[0])
        goto END;


    //Fixme: scale to dst width
    data = frame->data[0];
    //glViewport(0, 0, width, height);  
    gles2_bindTexture(gl_ctx.g_texYId, data, width, height);  
    gles2_bindTexture(gl_ctx.g_texUId, data + width * height, width/2, height/2);  
    gles2_bindTexture(gl_ctx.g_texVId, data + width * height * 5 / 4, width/2, height/2);  

    gles2_renderFrame();
    if(frame->data[0])
        free(frame->data[0]);
    frame->data[0] = NULL;
    gl_ctx.invalid_frame = 0;
END:
    dt_unlock(&mutex);
    return 0;
}

} // end namespace android
