LOCAL_PATH:= $(call my-dir)

GLOBAL_CFLAGS := -DHAVE_ANDROID_OS -DHAVE_PTHREADS -DHAVE_SYS_UIO_H -Wno-psabi
##########################################################

include $(CLEAR_VARS)
LOCAL_SRC_FILES:=               \
               iniparser/iniparser.cpp	 \
               iniparser/dictionary.cpp

LOCAL_C_INCLUDES :=  $(LOCAL_PATH)/iniparser/

LOCAL_MODULE:= libiniparser
include $(BUILD_STATIC_LIBRARY)

#########################################################

INCLUDE_PATH := \
	$(LOCAL_PATH)/include/frameworks/av/media/libstagefright \
	$(LOCAL_PATH)/include/frameworks/av/include \
	$(LOCAL_PATH)/include/frameworks/native/include \
    $(LOCAL_PATH)/include/frameworks/native/include/media/openmax \
    $(LOCAL_PATH)/include/frameworks/av/media/libstagefright/mpeg2ts \
    $(LOCAL_PATH)/include/system/core/include \
    $(LOCAL_PATH)/include/hardware/libhardware/include \
    
        
include $(CLEAR_VARS)

LOCAL_CFLAGS := $(GLOBAL_CFLAGS)

LOCAL_SRC_FILES:= \
		wfd.cpp							\
        ANetworkSession.cpp             \
        Parameters.cpp                  \
        ParsedMessage.cpp               \
        sink/LinearRegression.cpp       \
        sink/RTPSink.cpp                \
        sink/TunnelRenderer.cpp         \
        sink/WifiDisplaySink.cpp        \

LOCAL_C_INCLUDES:= $(INCLUDE_PATH)
LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/iniparser/

LOCAL_LDLIBS := $(LOCAL_PATH)/ext_lib/libbinder.so
LOCAL_LDLIBS += $(LOCAL_PATH)/ext_lib/libcutils.so
LOCAL_LDLIBS += $(LOCAL_PATH)/ext_lib/libgui.so
LOCAL_LDLIBS += $(LOCAL_PATH)/ext_lib/libmedia.so
LOCAL_LDLIBS += $(LOCAL_PATH)/ext_lib/libstagefright.so
LOCAL_LDLIBS += $(LOCAL_PATH)/ext_lib/libstagefright_foundation.so
LOCAL_LDLIBS += $(LOCAL_PATH)/ext_lib/libui.so
LOCAL_LDLIBS += $(LOCAL_PATH)/ext_lib/libutils.so

LOCAL_SHARED_LIBRARIES:= \
        libbinder                       \
        libcutils                       \
        libgui                          \
        libmedia                        \
        libstagefright                  \
        libstagefright_foundation       \
        libui                           \
	libutils                        

LOCAL_STATIC_LIBRARIES := libiniparser
LOCAL_MODULE:= libwfd

include $(BUILD_SHARED_LIBRARY)
