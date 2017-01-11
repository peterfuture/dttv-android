//
// Created by dttv on 16-5-3.
//

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <android/log.h>
#include "native_log.h"

#define  TAG  "gl2_yuv"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "gl_util.h"
#include "gl_yuv.h"
#include "../../../../3rd/libdtp/include/dtp_av.h"
#include "android_dtplayer.h"
#include "jni_utils.h"

using namespace android;

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGV("GL %s = %s\n", name, v);
}

const char gVertextShader[] = {
        "attribute vec4 aPosition;\n"
                "attribute vec2 aTextureCoord;\n"
                "varying vec2 vTextureCoord;\n"
                "void main() {\n"
                "  gl_Position = aPosition;\n"
                "  vTextureCoord = aTextureCoord;\n"
                "}\n"
};

// The fragment shader.
// Do YUV to RGB565 conversion.
static const char gFragmentShader[] = {
        "precision mediump float;\n"
                "uniform sampler2D Ytex;\n"
                "uniform sampler2D Utex,Vtex;\n"
                "varying vec2 vTextureCoord;\n"
                "void main(void) {\n"
                "  float nx,ny,r,g,b,y,u,v;\n"
                "  mediump vec4 txl,ux,vx;"
                "  nx=vTextureCoord[0];\n"
                "  ny=vTextureCoord[1];\n"
                "  y=texture2D(Ytex,vec2(nx,ny)).r;\n"
                "  u=texture2D(Utex,vec2(nx,ny)).r;\n"
                "  v=texture2D(Vtex,vec2(nx,ny)).r;\n"

                //"  y = v;\n"+
                "  y=1.1643*(y-0.0625);\n"
                "  u=u-0.5;\n"
                "  v=v-0.5;\n"

                "  r=y+1.5958*v;\n"
                "  g=y-0.39173*u-0.81290*v;\n"
                "  b=y+2.017*u;\n"
                "  gl_FragColor=vec4(r,g,b,1.0);\n"
                "}\n"
};

static GLuint gProgram;
static GLuint positionHandle;
static GLuint textureHandle;

static GLuint g_textureIds[3];
static GLuint g_textureWidth = 0;
static GLuint g_textureHeight = 0;

static GLuint g_windowWidth = 0;
static GLuint g_windowHeight = 0;

const char g_indices[] = {0, 3, 2, 0, 2, 1};

const GLfloat g_vertices[20] = {
        // X, Y, Z, U, V
        -1, -1, 0, 0, 1, // Bottom Left
        1, -1, 0, 1, 1, //Bottom Right
        1, 1, 0, 1, 0, //Top Right
        -1, 1, 0, 0, 0
}; //Top Left

void setupTextures(uint8_t *data, GLsizei width, GLsizei height) {
    glGenTextures(3, g_textureIds); //Generate  the Y, U and V texture

    GLuint currentTextureId = g_textureIds[0]; // Y
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTextureId);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE,
                 (const GLvoid *) data);

    currentTextureId = g_textureIds[1]; // U
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, currentTextureId);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    uint8_t *uComponent = data + width * height;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, (const GLvoid *) uComponent);

    currentTextureId = g_textureIds[2]; // V
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, currentTextureId);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    uint8_t *vComponent = uComponent + (width * height) / 4;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, (const GLvoid *) vComponent);
    checkGlError("SetupTextures");

    LOGV("setupTextures ok");
}


void UpdateTextures(uint8_t *data, GLsizei width, GLsizei height) {
    GLuint currentTextureId = g_textureIds[0]; // Y
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTextureId);
    checkGlError("UpdateTextures Y Bind");
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE,
                    GL_UNSIGNED_BYTE, (const GLvoid *) data);
    checkGlError("UpdateTextures Y");
    currentTextureId = g_textureIds[1]; // U
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, currentTextureId);
    checkGlError("UpdateTextures U Bind");
    uint8_t *uComponent = data + width * height;
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2,
                    GL_LUMINANCE, GL_UNSIGNED_BYTE, (const GLvoid *) uComponent);
    checkGlError("UpdateTextures U");

    currentTextureId = g_textureIds[2]; // V
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, currentTextureId);
    checkGlError("UpdateTextures V Bind");
    uint8_t *vComponent = uComponent + (width * height) / 4;
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2,
                    GL_LUMINANCE, GL_UNSIGNED_BYTE, (const GLvoid *) vComponent);
    checkGlError("UpdateTextures V");
    checkGlError("UpdateTextures");
}

bool yuv_setupGraphics(int w, int h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    g_windowWidth = (GLuint) w;
    g_windowHeight = (GLuint) h;
    LOGV("setupGraphics(%d, %d)", w, h);

    int maxTextureImageUnits[2];
    int maxTextureSize[2];
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, maxTextureImageUnits);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, maxTextureSize);
    LOGV("%s: number of textures %d, size %d", __FUNCTION__, (int) maxTextureImageUnits[0],
         (int) maxTextureSize[0]);

    gProgram = createProgram(gVertextShader, gFragmentShader);
    if (!gProgram) {
        LOGV("Could not create program.");
        return false;
    }

    positionHandle = glGetAttribLocation(gProgram, "aPosition");
    checkGlError("glGetAttribLocation");
    if (positionHandle == -1) {
        LOGV("%s: Could not get aPosition handle", __FUNCTION__);
        return -1;
    }

    textureHandle = glGetAttribLocation(gProgram, "aTextureCoord");
    checkGlError("glGetAttribLocation");
    if (textureHandle == -1) {
        LOGV("%s: Could not get aTextureCoord handle", __FUNCTION__);
        return -1;
    }

    // set the vertices array in the shader
    // _vertices contains 4 vertices with 5 coordinates.
    // 3 for (xyz) for the vertices and 2 for the texture
    glVertexAttribPointer(positionHandle, 3, GL_FLOAT, false, 5 * sizeof(GLfloat), g_vertices);
    checkGlError("glVertexAttribPointer aPosition");

    glEnableVertexAttribArray(positionHandle);
    checkGlError("glEnableVertexAttribArray positionHandle");

    // set the texture coordinate array in the shader
    // _vertices contains 4 vertices with 5 coordinates.
    // 3 for (xyz) for the vertices and 2 for the texture
    glVertexAttribPointer(textureHandle, 2, GL_FLOAT, false, 5 * sizeof(GLfloat), &g_vertices[3]);
    checkGlError("glVertexAttribPointer maTextureHandle");
    glEnableVertexAttribArray(textureHandle);
    checkGlError("glEnableVertexAttribArray textureHandle");


    glUseProgram(gProgram);
    int i = glGetUniformLocation(gProgram, "Ytex");
    checkGlError("glGetUniformLocation");
    glUniform1i(i, 0); /* Bind Ytex to texture unit 0 */
    checkGlError("glUniform1i Ytex");

    i = glGetUniformLocation(gProgram, "Utex");
    checkGlError("glGetUniformLocation Utex");
    glUniform1i(i, 1); /* Bind Utex to texture unit 1 */
    checkGlError("glUniform1i Utex");

    i = glGetUniformLocation(gProgram, "Vtex");
    checkGlError("glGetUniformLocation");
    glUniform1i(i, 2); /* Bind Vtex to texture unit 2 */
    checkGlError("glUniform1i");

    glViewport(0, 0, w, h);
    checkGlError("glViewport");

    LOGV("YUV setup graphics ok");
    return true;
}

static dt_av_frame_t g_frame;
static lock_t mutex;
static int frame_valid = 0;
static int g_inited = 0;
static android::DTPlayer *g_dtp;

void yuv_dttv_init() {
    lock_init(&mutex, NULL);
    memset(&g_frame, 0, sizeof(dt_av_frame_t));
    frame_valid = 0;
    g_inited = 1;
    g_dtp = NULL;

    // Fixme -
    g_windowWidth = g_windowHeight = 0;
    g_textureHeight = g_textureWidth = 0;

    LOGV("yuv dttv init\n");
}

void yuv_reg_player(void *mp) {
    g_dtp = (android::DTPlayer *) mp;
    LOGV("register dtplayer to glesv2\n");
}

int yuv_update_frame(dt_av_frame_t *frame) {
    int ret = 0;
    if (g_inited == 0) {
        if (frame->data[0]) {
            free(frame->data[0]);
        }
        LOGV("Not inited yet");
        return 0;
    }

    lock(&mutex);
    // check frame not displayed
    if (frame_valid == 1 && g_frame.data) {
        free(g_frame.data[0]);
    }
    memcpy(&g_frame, frame, sizeof(dt_av_frame_t));
    frame_valid = 1;
    unlock(&mutex);

    if (!g_dtp) {
        LOGV("mp null \n");
        return 0;
    }
    g_dtp->Notify(MEDIA_FRESH_VIDEO);
    LOGV("Refresh one frame");
    return 0;
}


void yuv_renderFrame() {
    if (frame_valid != 1) {
        return;
    }
    lock(&mutex);
    uint8_t *data = (uint8_t *) (g_frame.data[0]);
    int width = g_frame.width;
    int height = g_frame.height;

    glUseProgram(gProgram);
    checkGlError("glUseProgram");
    if (g_textureWidth != g_windowWidth ||
        g_textureHeight != g_windowHeight) {
        LOGV("TEXTREUE w:%d h:%d . [%d:%d] \n", (int) g_textureWidth, (int) g_textureHeight,
             (int) g_windowWidth, (int) g_windowHeight);
        setupTextures(data, width, height);
        g_textureWidth = g_windowWidth;
        g_textureHeight = g_windowHeight;
    } else {
        UpdateTextures(data, width, height);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, g_indices);
    checkGlError("glDrawArrays");
    free(data);
    frame_valid = 0;
    memset(&g_frame, 0, sizeof(dt_av_frame_t));
    unlock(&mutex);
    LOGV("Draw one frame");
}
