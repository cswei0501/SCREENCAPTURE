LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

NDK_BUILDER := 1
#ANDROID_RESOURCE_BUILDER := 1
#AudioRecordFlag := 1
#OPHONE_OR_ASUS := 1
#APUSONE := 1
TAIFA_TECH := 1


LOCAL_SRC_FILES:= \
	cidana_screencap_Capture.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libskia \
	libmedia \
    libui 

LOCAL_MODULE:= libscreencapture

ifeq ($(NDK_BUILDER), 1)
LOCAL_CFLAGS += -DNDK_BUILDER
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
endif

ifeq ($(ANDROID_RESOURCE_BUILDER), 1)
LOCAL_CFLAGS += -DANDROID_RESOURCE_BUILDER
endif

ifeq ($(AudioRecordFlag), 1)
LOCAL_CFLAGS += -DAudioRecordFlag
endif

ifeq ($(APUSONE), 1)
LOCAL_CFLAGS += -DAPUSONE
endif

ifeq ($(TAIFA_TECH), 1)
LOCAL_CFLAGS += -DTAIFA_TECH
endif

ifeq ($(OPHONE_OR_ASUS), 1)
LOCAL_LDFLAGS += $(LOCAL_PATH)/libJpegEnc_arm9.a
LOCAL_LDFLAGS += $(LOCAL_PATH)/libJpegEncComponent_arm9.a
LOCAL_LDFLAGS += $(LOCAL_PATH)/arm/CSCLib.a
LOCAL_LDFLAGS += $(LOCAL_PATH)/arm/CSCLib_arm.a
LOCAL_CFLAGS += -DOPHONE_OR_ASUS
else
LOCAL_LDFLAGS += $(LOCAL_PATH)/libJpegEnc_arm11.a
LOCAL_LDFLAGS += $(LOCAL_PATH)/libJpegEncComponent_arm11.a
LOCAL_LDFLAGS += $(LOCAL_PATH)/neon/CSCLib.a
LOCAL_LDFLAGS += $(LOCAL_PATH)/neon/CSCLib_neon.a
#LOCAL_LDFLAGS += $(LOCAL_PATH)/arm/CSCLib.a
#LOCAL_LDFLAGS += $(LOCAL_PATH)/arm/CSCLib_arm.a
endif

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

include $(BUILD_SHARED_LIBRARY)
