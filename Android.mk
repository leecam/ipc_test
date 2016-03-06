#
# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ipc_test
LOCAL_CFLAGS := -Wall -Werror -Wno-unused-parameter
LOCAL_CPP_EXTENSION := .cc
LOCAL_SHARED_LIBRARIES := \
  libbinder \
  libutils \

LOCAL_SRC_FILES := \
  ipc_test.cc \

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
#LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE := ipc_sockets
LOCAL_CFLAGS := -Wall -Werror -Wno-unused-parameter
LOCAL_CPP_EXTENSION := .cc
LOCAL_SHARED_LIBRARIES := \
  libbinder \
  libutils \
  libc \

LOCAL_SRC_FILES := \
  ipc_sockets.cc \

include $(BUILD_EXECUTABLE)
