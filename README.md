# dttv-android

dttv-android is an open-sourced project, published under GPLv3 for individual/personal users .

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

## Introduction

dttv-android is a videoplayer for android platform.
based on [dtplayer].

## How to use
```
allprojects {
    repositories {
        jcenter()
    }
}

dependencies {
    compile 'app.dttv:dttvlib:0.0.1'
}
```

## Build

>
* download dtplayer so zip [here](https://sourceforge.net/projects/dttv/files/lib.zip/download) and unzip to dttv/dttvlib/distribution/libdtp/
* build dttv with android studio
* apk: dttv/dttv-samples/build/outputs/apk/dttv-samples-debug.apk.
* Get prebuild [here](https://sourceforge.net/projects/dttv/files/dttv-samples-debug.apk/download)


## Deps

>
* [dtplayer]
* [ffmpeg]
