//
// Created by dttv on 16-5-2.
//

#ifndef GLES2JNI_GL_UTIL_H
#define GLES2JNI_GL_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

void checkGlError(const char* op);
GLuint loadShader(GLenum shaderType, const char* pSource);
GLuint createProgram(const char* pVertexSource, const char* pFragmentSource);

#endif //GLES2JNI_GL_UTIL_H
