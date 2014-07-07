#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <stdlib.h>

#include <GLES/gl.h>
#include <GLES/glext.h>

#include "dtplayer_api.h"
#include "render_android.h"
#include "dt_lock.h"

#define DEBUG_TAG "dttvJni"

static void *handle;
extern int android_audio_init(int channels, int samplerate);

#define GLRENDER_STATUS_IDLE 0
#define GLRENDER_STATUS_RUNNING 1
#define GLRENDER_STATUS_QUIT 2

#define TEXTURE_WIDTH 320
#define TEXTURE_HEIGHT 240

#define S_PIXELS_SIZE (sizeof(s_pixels[0]) * TEXTURE_WIDTH * TEXTURE_HEIGHT)
#define RGB565(r, g, b)  (((r) << (5+6)) | ((g) << 6) | (b))
/* disable these capabilities. */
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
    int frame_size;
	int width;
	int height;
	int status;
    int invalid_frame;

    GLuint s_texture;
    dt_lock_t mutex;
}gl_ctx_t;

static gl_ctx_t gl_ctx;

int native_ui_init(JNIEnv * env, jobject this, jint w, jint h)
{
	memset(&gl_ctx,0,sizeof(gl_ctx_t));
	gl_ctx.width = w;
	gl_ctx.height = h;
	gl_ctx.frame_size = w*h*2;
	gl_ctx.frame = (uint16_t *)malloc(gl_ctx.frame_size); // rgb565
	if(gl_ctx.frame == NULL)
		return -1;
    
    dt_lock_init (&gl_ctx.mutex, NULL);
    gl_ctx.status = GLRENDER_STATUS_RUNNING;

	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Native UI Init OK, size:%d \n",gl_ctx.frame_size);
    return 0;
}

int getActivityWidth()
{
    return gl_ctx.width;
}

int getActivityHeight()
{
    return gl_ctx.height;
}

static void check_gl_error(const char* op)
{
	GLint error;
	for (error = glGetError(); error; error = glGetError())
		__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG,"after %s() glError (0x%x)\n", op, error);
}

int update_frame(uint8_t *buf,int size)
{
    if(size > gl_ctx.frame_size)
        return 0;

    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "update_frame, size:%d \n",gl_ctx.frame_size);
    dt_lock (&gl_ctx.mutex);
    memcpy((uint8_t *)gl_ctx.frame,buf,gl_ctx.frame_size);
    gl_ctx.invalid_frame = 1;
    dt_unlock (&gl_ctx.mutex);
}


void native_gl_resize(JNIEnv *env, jclass clazz, jint w, jint h)
{
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG,"native_gl_resize %d %d", w, h);
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
	int rect[4] = {0, TEXTURE_HEIGHT, TEXTURE_WIDTH, -TEXTURE_HEIGHT};
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);
	check_gl_error("glTexParameteriv");
}

static update_pixel_test()
{
    uint16_t *pixels = gl_ctx.frame;
    int x, y;
    int s_x = 50;
    int s_y = 100;
    memset(pixels, 0, 320*240*2);
	/* fill in a square of 5 x 5 at s_x, s_y */
	for (y = s_y; y < s_y + 5; y++) {
		for (x = s_x; x < s_x + 5; x++) {
			int idx = x + y * gl_ctx.width;
			pixels[idx++] = RGB565(31, 31, 0);
		}
	}
}

int native_disp_frame(JNIEnv * env, jobject this)
{
    if(gl_ctx.status == GLRENDER_STATUS_IDLE)
        return 0;
    if(gl_ctx.invalid_frame == 0)
        return 0;

    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "start disp frame\n");
    //display one frame
    dt_lock (&gl_ctx.mutex);
	
    glClear(GL_COLOR_BUFFER_BIT);
    //update_pixel_test();
	glTexImage2D(GL_TEXTURE_2D,		/* target */
			0,			/* level */
			GL_RGB,			/* internal format */
			gl_ctx.width,		/* width */
			gl_ctx.height,		/* height */
			0,			/* border */
			GL_RGB,			/* format */
			GL_UNSIGNED_SHORT_5_6_5,/* type */
			gl_ctx.frame);		/* pixels */
	glDrawTexiOES(0, 0, 0, gl_ctx.width, gl_ctx.height);
    gl_ctx.invalid_frame = 0;
    
    dt_unlock (&gl_ctx.mutex);
}

int native_ui_stop(JNIEnv * env, jobject this)
{
	if(gl_ctx.frame)
		free(gl_ctx.frame);
	memset(&gl_ctx,0,sizeof(gl_ctx_t));
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Native UI Stop OK \n");
    return 0;
}

int native_playerStart(JNIEnv * env, jobject this, jstring url)
{
    jboolean isCopy;
    char * file_name = (*env)->GetStringUTFChars(env, url, &isCopy);
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Receive start cmd, file [%s] size:%d ",file_name,strlen(file_name));


    render_init();

    dtplayer_para_t para;

    para.no_audio = para.no_video = para.no_sub = -1;
	para.height = para.width = -1;
	para.loop_mode = 0;
	para.audio_index = para.video_index = para.sub_index = -1;
	para.update_cb = NULL;
	para.sync_enable = -1;

	para.file_name = file_name;
	//para.update_cb = (void *) update_cb;
	para.no_audio=1;
	//para.no_video=1;
	para.width = gl_ctx.width;
	para.height = gl_ctx.height;

    handle = dtplayer_init(&para);
    if (!handle)
        return -1;

    //get media info
	dt_media_info_t info;
	int ret = dtplayer_get_mediainfo(handle, &info);
	if(ret < 0)
	{
	    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Get mediainfo failed, quit \n");
		return -1;
	}
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Get Media Info Ok,filesize:%lld fulltime:%lld S \n",info.file_size,info.duration);

	dtplayer_start (handle);
    return 0;
}

int native_playerPause(JNIEnv * env, jobject this)
{
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Receive pause cmd");
	dtplayer_pause(handle);
    return 0;
}
int native_playerResume(JNIEnv * env, jobject this)
{
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Receive resume cmd");
	dtplayer_resume(handle);
    return 0;
}

int native_playerSeekTo(JNIEnv * env, jobject this, jint pos)
{
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Receive seek cmd, seek to %d ",pos);
    return 0;
}

int native_playerStop(JNIEnv * env, jobject this)
{
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Receive pause cmd");
    if(handle)
    	dtplayer_stop (handle);
    handle = NULL;
    return 0;
}
