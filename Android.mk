LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := audio_path_test
# LOCAL_MODULE_TAGS := userdebug debug
LOCAL_SRC_FILES := audio_path_test.cpp
LOCAL_SHARED_LIBRARIES := \
	libcutils \
	liblog \
	libutils \
	libaudioclient

LOCAL_C_INCLUDES += frameworks/native/include/ \
	system/core/include/utils/

LOCAL_CFLAGS += -Wno-unused-parameter -Wno-unused-variable

include $(BUILD_EXECUTABLE)
