LOCAL_PATH := $(call my-dir)

#USE_PREBUILD_WFDLIB := 1

ifdef USE_PREBUILD_WFDLIB
###########################
include $(CLEAR_VARS)
LOCAL_SRC_FILES := ext_lib/libwfd.so
LOCAL_MODULE := libwfd
include $(PREBUILT_SHARED_LIBRARY)

###########################

endif

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/wifidisplay

LOCAL_MODULE    := libwfd_jni
LOCAL_SRC_FILES += wfd_jni.cpp
LOCAL_SRC_FILES += dp.cpp

LOCAL_LDLIBS    += -landroid -llog

LOCAL_SHARED_LIBRARIES := libwfd
LOCAL_SHARED_LIBRARIES += libutils

LOCAL_CFLAGS := -Wno-psabi
include $(BUILD_SHARED_LIBRARY)

#######################
ifndef USE_PREBUILD_WFDLIB
include $(LOCAL_PATH)/wifidisplay/Android.mk
endif
