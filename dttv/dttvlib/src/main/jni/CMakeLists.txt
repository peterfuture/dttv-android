
#
# Copyright (C) The Android Open Source Project
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

cmake_minimum_required(VERSION 3.4.1)

# configure import libs
set(distribution_DIR ${CMAKE_SOURCE_DIR}/../../../distribution)

# static lib import
# add_library(lib_dtp STATIC IMPORTED)
# set_target_properties(lib_dtp PROPERTIES IMPORTED_LOCATION
# ${distribution_DIR}/libdtp/lib/${ANDROID_ABI}/libdtp.a)

# shared lib import
add_library(lib_dtp SHARED IMPORTED)
set_target_properties(lib_dtp PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/libdtp/lib/${ANDROID_ABI}/libdtp.so)

# build application's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

# jni source code
aux_source_directory( . JNISOURCES)
aux_source_directory( ./plugin/ PLUGINSOURCES)

set (SOURCE
    ${JNISOURCES}
    ${PLUGINSOURCES}
)

add_library(dttv_jni SHARED
            ${SOURCE})

target_include_directories(dttv_jni PRIVATE
                           ${CMAKE_SOURCE_DIR}/
                           ${CMAKE_SOURCE_DIR}/plugin/
                           ${distribution_DIR}/libdtp/include)

target_link_libraries(dttv_jni
                      android
                      lib_dtp
                      GLESv2
                      log)
