dttv-android
============

dttv-android is an open-sourced project, published under GPLv3 for individual/personal users .

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

Introduction
========

dttv-android is a videoplayer for android platform.

developed with dtplayer(android version) and ffmpeg.

Build
========

1 Setup ndk & sdk & toolchain

export ANDROID_SDK=/PATH/TO/adt_x64/sdk
export ANDROID_NDK=/PATH/TO/android-ndk-r9
export ANDROID_TOOL_CHAIN=/opt/toolchains/android-ndk-r8d-android-14-arm-linux-androideabi-4.6/
export ANDROID_TOOL_CHAIN_BIN=/opt/toolchains/android-ndk-r8d-android-14-arm-linux-androideabi-4.6/bin/

export PATH=$PATH:$ANDROID_TOOL_CHAIN_BIN
export PATH=$PATH:$ANDROID_NDK

$ANDROID_NDK/build/tools/make-standalone-toolchain.sh --platform=android-14 --install-dir=$ANDROID_TOOL_CHAIN

2 Setup AOSP

export AOSP_TREE = /PATH/TO/AOSP
export AOSP_OUT = $(AOSP_TREE)/out/target/product/platform/

3 ndk-build

cd dttv/jni & ndk-build (refer to readme under jni)

4 build apk with eclipse


REF REPO
========

dttv/jni/libs/libdtp.a --> build from https://github.com/peterfuture/dtplayer_c
dttv/jni/libs/libamffmpeg.a --> build from https://github.com/peterfuture/FFmpeg-Android

[comments] no need user to update


Test
========

...

[Help]
=========

Email: peter_future@outlook.com 

QQ: peter_future@outlook.com

blog: http://blog.csdn.net/dtplayer
