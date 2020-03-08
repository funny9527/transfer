LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_LDLIBS :=-llog

LOCAL_MODULE    := transfer
LOCAL_SRC_FILES := init.cpp \
                   net/client.cpp \
                   net/server.cpp

include $(BUILD_SHARED_LIBRARY)
