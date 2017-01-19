#ifndef DTTV_JNI_H
#define DTTV_JNI_H

#include <jni.h>

class dttvListenner {
public:
    dttvListenner(JNIEnv *env, jobject thiz, jobject weak_thiz);

    ~dttvListenner();

    int notify(int, int ext1 = 0, int ext2 = 0);

private:
    dttvListenner();

    jclass mClass;   // Reference to DtPlayer class
    jobject mObject; // weak ref to DtPlayer Java object to call on
};

#endif
