LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := testscreencapture
LOCAL_CERTIFICATE := platform

LOCAL_JNI_SHARED_LIBRARIES := libscreencapture

include $(BUILD_PACKAGE)
