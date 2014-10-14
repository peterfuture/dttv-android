#ifndef ANDROID_DTPLAYER_H
#define ANDROID_DTPLAYER_H

#include <jni.h>

class dtpListenner{
    public:
        dtpListenner(JNIEnv *env, jobject thiz, jobject weak_thiz);
        ~dtpListenner();
        int notify(int, int ext1 = 0, int ext2 = 0);
    private:
        dtpListenner();
        jclass mClass;   // Reference to DtPlayer class
        jobject mObject; // weak ref to DtPlayer Java object to call on
};

#endif
