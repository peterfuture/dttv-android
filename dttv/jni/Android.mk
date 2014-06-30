LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := dtp_jni
LOCAL_SRC_FILES := register_natives.c dtp_native.c render_android.c ao_android.cpp vo_android.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_LDLIBS    := -llog -L$(LOCAL_PATH)/libs -ldtp -lffmpeg -lz

#android env
LOCAL_C_INCLUDES += $(AOSP_TREE)/frameworks/av/include/       #media
LOCAL_C_INCLUDES += $(AOSP_TREE)/system/core/include          #cutils
LOCAL_C_INCLUDES += $(AOSP_TREE)/hardware/libhardware/include #hardware
LOCAL_C_INCLUDES += $(AOSP_TREE)/frameworks/native/include    #utils

LOCAL_LDLIBS     += -L$(AOSP_OUT)/system/lib -lmedia  -lutils -lGLESv1_CM

include $(BUILD_SHARED_LIBRARY)
