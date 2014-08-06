#ifndef ANDROID_DTPLAYER_H
#define ANDROID_DTPLAYER_H

#include <jni.h>

class dtpListenner{
    public:
        dtpListenner(JNIEnv *env, jobject thiz);
        ~dtpListenner();
        int notify(int);
    private:
        dtpListenner();
        jclass mClass;
        jobject mObj;
        jmethodID notify_cb;
};

#endif
