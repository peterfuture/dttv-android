LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_LDLIBS := -llog

LOCAL_MODULE    := dtp_jni
LOCAL_SRC_FILES := dtp_jni.c

include $(BUILD_SHARED_LIBRARY)
