LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#video 1: neon optimized; 2: not optimized;
VIDEO_PARTICULARLY_PROCESS := 1

LOCAL_SRC_FILES:= \
	capture_encoder.cpp \

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libskia \
	libmedia \
    libui 

ifeq "$(VIDEO_PARTICULARLY_PROCESS)" "1"
	LOCAL_LDFLAGS += $(LOCAL_PATH)/libJpegEnc_arm11.a
	LOCAL_LDFLAGS += $(LOCAL_PATH)/libJpegEncComponent_arm11.a
	LOCAL_LDFLAGS += $(LOCAL_PATH)/neon/CSCLib.a
	LOCAL_LDFLAGS += $(LOCAL_PATH)/neon/CSCLib_neon.a
else ifeq "$(VIDEO_PARTICULARLY_PROCESS)"  "2"
	LOCAL_LDFLAGS += $(LOCAL_PATH)/libJpegEnc_arm11.a
	LOCAL_LDFLAGS += $(LOCAL_PATH)/libJpegEncComponent_arm11.a
	LOCAL_LDFLAGS += $(LOCAL_PATH)/arm/CSCLib.a
	LOCAL_LDFLAGS += $(LOCAL_PATH)/arm/CSCLib_arm.a
endif
	
LOCAL_MODULE:= captureEncoder

LOCAL_MODULE_TAGS := eng

LOCAL_C_INCLUDES += \
	external/skia/include/core \
	external/skia/include/effects \
	external/skia/include/images \
	external/skia/src/ports \
	external/skia/include/utils
	
LOCAL_C_INCLUDES += $(JNI_H_INCLUDE)

OUTPUT_PATH := $(LOCAL_PATH)

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_PATH := $(OUTPUT_PATH)/release_arm
LOCAL_UNSTRIPPED_PATH := $(OUTPUT_PATH)/unstripped_arm

include $(BUILD_EXECUTABLE)
