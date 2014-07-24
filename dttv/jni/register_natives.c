#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dtp_native.h"
#include "dtp_native_api.h"

#ifndef NELEM
#define NELEM(x) ((int)(sizeof(x) / sizeof((x)[0])))
#endif

static const char *s_class_path_name = "dttv/app/PlayActivity";

static JNINativeMethod s_methods[] = {
	{"native_gl_resize", "(II)V", (void*) native_gl_resize},
	{"native_ui_init", "(II)I", (void*) native_ui_init},
	{"native_disp_frame", "()I", (void*) native_disp_frame},
	{"native_ui_stop", "()I", (void*) native_ui_stop},

	{"native_playerStart", "(Ljava/lang/String;)I", (void*) native_playerStart},
	{"native_playerPause", "()I", (void*) native_playerPause},
	{"native_playerResume", "()I", (void*) native_playerResume},
	{"native_playerSeekTo", "(I)I", (void*) native_playerSeekTo},
	{"native_playerStop", "()I", (void*) native_playerStop},

	{"native_getCurrentPostion", "()I", (void*) native_getCurrentPostion},
	{"native_getDuration", "()I", (void*) native_getDuration},
	{"native_getPlayerStatus", "()I", (void*) native_getPlayerStatus},


    //For New API
	{"native_setDataSource", "(Ljava/lang/String;)I", (void*) dtp_setDataSource},
	{"native_prePare", "()I", (void*) dtp_prePare},
	{"native_prePareAsync", "()I", (void*) dtp_prepareAsync},
	{"native_start", "()I", (void*) dtp_start},
	{"native_pause", "()I", (void*) dtp_pause},
	{"native_seekTo", "(I)I", (void*) dtp_seekTo},
	{"native_stop", "()I", (void*) dtp_stop},
	{"native_release", "()I", (void*) dtp_release},
	{"native_reset", "()I", (void*) dtp_reset},
	{"native_getVideoWidth", "()I", (void*) dtp_getVideoWidth},
    {"native_getVideoHeight", "()I", (void*) dtp_getVideoHeight},
    {"native_isPlaying", "()I", (void*) dtp_isPlaying},
    {"native_getCurrentPosition", "()I", (void*) dtp_getCurrentPosition},
    {"native_getDuration", "()I", (void*) dtp_getDuration},

};


static int register_native_methods(JNIEnv* env,
		const char* class_name,
		JNINativeMethod* methods,
		int num_methods)
{
	jclass clazz;

	clazz = (*env)->FindClass(env, class_name);
	if (clazz == NULL) {
		fprintf(stderr, "Native registration unable to find class '%s'\n",
				class_name);
		return JNI_FALSE;
	}
	if ((*env)->RegisterNatives(env, clazz, methods, num_methods) < 0) {
		fprintf(stderr, "RegisterNatives failed for '%s'\n", class_name);
		return JNI_FALSE;
	}

	return JNI_TRUE;
}


static int register_natives(JNIEnv *env)
{
	return register_native_methods(env,
			s_class_path_name,
			s_methods,
			NELEM(s_methods));
}

jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;

	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		fprintf(stderr, "ERROR: GetEnv failed\n");
		goto bail;
	}
	assert(env != NULL);

	if (register_natives(env) < 0) {
		fprintf(stderr, "ERROR: Exif native registration failed\n");
		goto bail;
	}

	/* success -- return valid version number */
	result = JNI_VERSION_1_4;
bail:
	return result;
}

