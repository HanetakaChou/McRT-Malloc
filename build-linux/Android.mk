#
# Copyright (C) YuqiaoZhang(HanetakaChou)
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

# https://developer.android.com/ndk/guides/android_mk

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := McRT-Malloc

LOCAL_SRC_FILES := \
	$(LOCAL_PATH)/../source/mcrt_current_thread_index.cpp \
	$(LOCAL_PATH)/../source/mcrt_malloc.cpp \
	$(LOCAL_PATH)/../source/mcrt_max_concurrency.cpp \
	$(LOCAL_PATH)/../source/mcrt_parallel_map.cpp \
	$(LOCAL_PATH)/../source/mcrt_parallel_reduce.cpp \
	$(LOCAL_PATH)/../source/mcrt_tick_count.cpp

LOCAL_CFLAGS :=

ifeq (armeabi-v7a,$(TARGET_ARCH_ABI))
LOCAL_ARM_MODE := arm
LOCAL_ARM_NEON := true
else ifeq (arm64-v8a,$(TARGET_ARCH_ABI))
LOCAL_CFLAGS +=
else ifeq (x86,$(TARGET_ARCH_ABI))
LOCAL_CFLAGS += -mf16c
LOCAL_CFLAGS += -mfma
LOCAL_CFLAGS += -mavx2
else ifeq (x86_64,$(TARGET_ARCH_ABI))
LOCAL_CFLAGS += -mf16c
LOCAL_CFLAGS += -mfma
LOCAL_CFLAGS += -mavx2
else
LOCAL_CFLAGS +=
endif

LOCAL_CFLAGS += -Wall
LOCAL_CFLAGS += -Werror=return-type

LOCAL_CFLAGS += -DTBB_SUPPRESS_DEPRECATED_MESSAGES=1
LOCAL_CFLAGS += -D__TBB_BUILD=1
LOCAL_CFLAGS += -D__TBBMALLOC_BUILD=1
LOCAL_CFLAGS += -D__TBB_NO_IMPLICIT_LINKAGE=1
LOCAL_CFLAGS += -D__TBBMALLOC_NO_IMPLICIT_LINKAGE=1
LOCAL_CFLAGS += -DUSE_PTHREAD

LOCAL_C_INCLUDES :=
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../thirdparty/Intel-TBB/include

LOCAL_CPPFLAGS := 
LOCAL_CPPFLAGS += -std=c++20

LOCAL_LDFLAGS :=
LOCAL_LDFLAGS += -Wl,--enable-new-dtags
LOCAL_LDFLAGS += -Wl,-rpath,\$$ORIGIN
LOCAL_LDFLAGS += -Wl,--version-script,$(LOCAL_PATH)/McRT-Malloc.map

LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += Intel-TBB

LOCAL_SHARED_LIBRARIES :=

include $(BUILD_SHARED_LIBRARY)
