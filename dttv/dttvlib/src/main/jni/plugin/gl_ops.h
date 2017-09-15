#ifndef GL_OPS_H
#define GL_OPS_H

#include <stdio.h>
#include <stdlib.h>

#include <GLES2/gl2.h>
#include <gl_filter.h>

void checkGlError(const char *op);
void printGLString(const char *name, GLenum s);
GLuint loadShader(GLenum shaderType, const char *pSource);
GLuint createProgram(const char *pVertexSource, const char *pFragmentSource);
int getHandle(GLuint program, char *name);

#endif