#ifndef DTTV_JNI_SURFACE_H
#define DTTV_JNI_SURFACE_H

#include <jni.h>
#include <android/native_window_jni.h>

int dttv_setup_gvm(void *vm);
ANativeWindow *dttv_nativewindow_from_surface(void *surface);

#endif
