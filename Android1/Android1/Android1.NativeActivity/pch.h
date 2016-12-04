//
// pch.h
// 標準システムのインクルード ファイルのヘッダー。
//
//プリコンパイル済みヘッダーを生成するためにビルド システムによって使用されました。
// pch.cpp は必要なく、pch.h がプロジェクトの一部であるすべての cpp ファイルに
// 自動的に含まれます。
//

#include <jni.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>

#include <android/log.h>
#include "android_native_app_glue.h"

#include "fft4g.hpp"
#define Range 2048

