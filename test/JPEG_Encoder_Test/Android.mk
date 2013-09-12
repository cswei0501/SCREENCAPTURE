LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=		\
	main.c

LOCAL_MODULE:= demo

LOCAL_SHARED_LIBRARIES := libui libutils
LOCAL_MODULE_TAGS := optional

LOCAL_LDFLAGS += $(LOCAL_PATH)/libJpegEnc_arm11.a
LOCAL_LDFLAGS += $(LOCAL_PATH)/libJpegEncComponent_arm11.a

include $(BUILD_EXECUTABLE)
