LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#choice only one condition if WIMO2 is "0"
COMMON			:=1
ZTE				:=0
HS				:=0
LENOVO			:=0
XIAOMI			:=0
SAMSUNG			:=0
ASUS			:=0
OPHONE			:=0
NEXUSONE_PAD	:=0
MTK				:=0
MOTO778			:=0

#choice only one condition
BUILDER_RESOURCE :=1
BUILDER_NDK 		:=0

AUDIO_SW		:=0

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libmedia \
	libui \
	libdl

#it need the lib if android 2.3 or upper version
LOCAL_SHARED_LIBRARIES += libgui

ifeq "$(NEXUSONE_PAD)" "1"
	LOCAL_CFLAGS += -DCYANOGEN_RESOURCE_BUILDER
	LOCAL_CFLAGS += -DNEXUSONE_AUDIO_MIXER
else ifeq "$(BUILDER_NDK)" "1"
	LOCAL_CFLAGS += -DNDK_RESOURCE_BUILDER
	LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
else ifeq "$(BUILDER_RESOURCE)" "1"
	LOCAL_CFLAGS += -DANDROID_RESOURCE_BUILDER
endif

ifeq "$(MTK)" "1"
	LOCAL_CFLAGS += -DMTK_CAPTURE_FRAME_BUFFER
endif

ifeq "$(AUDIO_SW)" "1"
	LOCAL_CFLAGS += -DAudioSWCapture
	ifeq "$(BUILDER_NDK)" "1"
		LOCAL_CFLAGS += -DNDK_AUDIO_RECORD
# for native audio
		LOCAL_LDLIBS    += -lOpenSLES
# for logging
		LOCAL_LDLIBS    += -llog
# for native asset manager
		LOCAL_LDLIBS    += -landroid
	endif
endif
	

LOCAL_STATIC_LIBRARIES += cpufeatures

LOCAL_SRC_FILES:= \
	./thirdParties/src/crc.cpp  \
	./src/capture_type.cpp \
	./src/capture_network.cpp \
	./src/checkCPU.cpp \
	./src/capture_video.cpp \
	./src/capture_audio.cpp \
	./src/cidana_screencap_Capture.cpp	

LOCAL_MODULE:= libciwimo

LOCAL_MODULE_TAGS := eng

LOCAL_CERTIFICATE := platform

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/Inc \
	$(LOCAL_PATH)/thirdParties/Inc \
	$(LOCAL_PATH)/lib/x86 \
	external/skia/include/core \
	external/skia/include/effects \
	external/skia/include/images \
	external/skia/src/ports \
	external/skia/include/utils \
	ndk/sources/android/cpufeatures
	
LOCAL_C_INCLUDES += $(JNI_H_INCLUDE)

OUTPUT_PATH := $(LOCAL_PATH)

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_PATH := $(OUTPUT_PATH)/release_arm
LOCAL_UNSTRIPPED_PATH := $(OUTPUT_PATH)/unstripped_arm
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= ./src/ci_screenshot.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libskia \
	libui \
	libgui 

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/Inc \
	external/skia/include/core \
	external/skia/include/effects \
	external/skia/include/images \
	external/skia/src/ports \
	external/skia/include/utils \
	frameworks/native/include/gui \
	frameworks/base/include/surfaceflinger
	
LOCAL_MODULE:= ciscreencap

LOCAL_MODULE_TAGS := eng
OUTPUT_PATH := $(LOCAL_PATH)
LOCAL_MODULE_PATH := $(OUTPUT_PATH)/release_arm
LOCAL_UNSTRIPPED_PATH := $(OUTPUT_PATH)/unstripped_arm
include $(BUILD_EXECUTABLE)
