LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := dtp_jni
LOCAL_SRC_FILES := android_jni.cpp
LOCAL_SRC_FILES += android_dtplayer.cpp
LOCAL_SRC_FILES += android_opengl.cpp
LOCAL_SRC_FILES += plugin/vo_android.c

LOCAL_CFLAGS += -D GL_GLEXT_PROTOTYPES -g
LOCAL_C_INCLUDES := $(DTP_TREE)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

#control
ENABLE_OPENSL = yes
ENABLE_AUDIOTRACK = no
ENABLE_ANDROID_OMX = no
ENABLE_OPENGL_V2 = yes
ENABLE_ANDROID_AE = no
ENABLE_DTAP = no

ifeq ($(ENABLE_OPENSL),yes)
	LOCAL_CFLAGS += -D ENABLE_OPENSL
	LOCAL_SRC_FILES += plugin/ao_opensl.c
endif
ifeq ($(ENABLE_AUDIOTRACK),yes)
	LOCAL_CFLAGS += -D ENABLE_AUDIOTRACK
	LOCAL_SRC_FILES += plugin/ao_audiotrack.cpp
endif
ifeq ($(ENABLE_ANDROID_OMX),yes)
	LOCAL_CFLAGS += -D ENABLE_ANDROID_OMX
	LOCAL_SRC_FILES += plugin/vd_stagefright.cpp
endif

ifeq ($(ENABLE_ANDROID_AE),yes)
	LOCAL_CFLAGS += -D ENABLE_ANDROID_AE
	LOCAL_SRC_FILES += plugin/ae_android.c
	LOCAL_LDLIBS += -lbundlewrapper -lreverbwrapper
endif

ifeq ($(ENABLE_OPENGL_V2),yes)
	LOCAL_CFLAGS += -D USE_OPENGL_V2
	LOCAL_LDLIBS += -lGLESv2
endif

ifeq ($(ENABLE_DTAP),yes)
	LOCAL_CFLAGS += -D ENABLE_DTAP
endif

LOCAL_LDLIBS    += -L$(DTP_ANDROID_LIB) -ldtp
LOCAL_LDLIBS    += -lOpenMAXAL -lmediandk
LOCAL_LDLIBS    += -llog
LOCAL_LDLIBS    += -landroid

include $(BUILD_SHARED_LIBRARY)
