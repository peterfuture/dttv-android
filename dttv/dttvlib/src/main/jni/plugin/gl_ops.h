#ifndef GL_OPS_H
#define GL_OPS_H

#include <stdio.h>
#include <stdlib.h>

#include <GLES2/gl2.h>
#include <gl_filter.h>
#include <dtp_av.h>

typedef int (*gl_ops_init)();
typedef int (*gl_ops_setup)(int w, int h);
typedef int (*gl_ops_update_frame)(dt_av_frame_t *frame);
typedef int (*gl_ops_render)();

typedef struct  {
    gl_ops_init init;
    gl_ops_setup setup;
    gl_ops_update_frame update;
    gl_ops_render render;
} gl_ops_t;

void checkGlError(const char *op);
void printGLString(const char *name, GLenum s);
GLuint loadShader(GLenum shaderType, const char *pSource);
GLuint createProgram(const char *pVertexSource, const char *pFragmentSource);
int getHandle(GLuint program, char *name);

#endif