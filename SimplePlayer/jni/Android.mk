LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := dtp_jni
LOCAL_SRC_FILES := dtp_jni.c render_android.c ao_android.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_LDLIBS    := -llog -L$(LOCAL_PATH)/libs -ldtp -lffmpeg -lz -lmedia
#LOCAL_STATIC_LIBRARIES := $(LOCAL_PATH)/libs/libdtp.a $(LOCAL_PATH)/libs/libffmpeg.a 

include $(BUILD_SHARED_LIBRARY)
