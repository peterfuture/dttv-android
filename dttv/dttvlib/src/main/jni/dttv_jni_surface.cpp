#include "dttv_jni_surface.h"

static JavaVM *surface_vm;

int dttv_setup_gvm(void *vm) {
    surface_vm = (JavaVM *)vm;
    return 0;
}

ANativeWindow *dttv_nativewindow_from_surface(void *surface) {
    JNIEnv *env = NULL;
    int isAttached = 0;
    if (surface_vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        if (surface_vm->AttachCurrentThread(&env, NULL) != JNI_OK) {
            return NULL;
        }
        isAttached = 1;
    }
    if(isAttached == 0)
        return NULL;
    jobject ref = env->NewGlobalRef((jobject)surface);
    ANativeWindow *window = ANativeWindow_fromSurface(env, (jobject)ref);

    if (isAttached) {
        surface_vm->DetachCurrentThread();
    }

    return window;
}