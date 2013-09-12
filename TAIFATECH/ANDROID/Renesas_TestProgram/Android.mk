LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

FORMAT := android
USING_STATIC_LIBRARY=1

LOCAL_SRC_FILES := test_camera.cpp

# set local parameters
ifeq ($(TARGET_ARCH),arm)
ARCH_PREFIX := arm
endif

ifeq ($(TARGET_ARCH),x86)
ARCH_PREFIX := linux_x86
endif

OUTPUT_PATH := $(LOCAL_PATH)/
	
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
		external/jpeg \
		external/emxx_jpeg \
		device/renesas/emev/omf    \
	      external/opencore/extern_libs_v2/khronos/openmax/include/
		
LOCAL_CFLAGS := -Wno-non-virtual-dtor

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_STATIC_LIBRARIES :=

#LD_LIBRARY_PATH := system/lib \
#	out/target/product/emev/obj/lib/ \
	


# set dynamic lib
LOCAL_SHARED_LIBRARIES := libjpegemxx 
#	libcamera \
	
#libhardware

#LOCAL_LDFLAGS += out/target/product/emev/obj/STATIC_LIBRARIES/libjpegemxx_intermediates/libjpegemxx.a

ifeq ($(USING_STATIC_LIBRARY), 1)
#LOCAL_LDFLAGS += $(LOCAL_PATH)/../../lib/ci_map200_h264dec_lnx.a
#LOCAL_LDFLAGS += $(LOCAL_PATH)/../../lib/ci_h264bpdec_armv4_lnx.a
endif

ifeq ($(USING_STATIC_LIBRARY), 0)
#LOCAL_CFLAGS+=-DUSING_SHARED_LIBRARY
endif

# set external static lib

# set binary output path
LOCAL_MODULE := renasasTest
LOCAL_MODULE_PATH := $(OUTPUT_PATH)/release_$(ARCH_PREFIX)
LOCAL_UNSTRIPPED_PATH := $(OUTPUT_PATH)/unstripped_$(ARCH_PREFIX)


include $(BUILD_EXECUTABLE)

