#include <gl_ops.h>
#include <dttv_jni_dtp.h>
#include <gl_render.h>

#define  TAG  "gl2_yuv"

using namespace android;

static gl_filter_type type = GL_FILTER_TYPE_YUV;

static GLuint gProgram;

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
        -1, 1, 0, 0, 0 //Top Left
};

static int FLOAT_SIZE_BYTES = 4;
static int VERTICES_DATA_POS_SIZE = 3;
static int VERTICES_DATA_UV_SIZE = 2;
static int VERTICES_DATA_STRIDE_BYTES = (VERTICES_DATA_POS_SIZE + VERTICES_DATA_UV_SIZE) * FLOAT_SIZE_BYTES;
static int VERTICES_DATA_POS_OFFSET = 0 * FLOAT_SIZE_BYTES;
static int VERTICES_DATA_UV_OFFSET = VERTICES_DATA_POS_OFFSET + VERTICES_DATA_POS_SIZE * FLOAT_SIZE_BYTES;

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

static void version()
{
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);
}

static int yuv_setupGraphics(int w, int h) {
    LOGV("setupGraphics(%d, %d)", w, h);

    g_windowWidth = (GLuint) w;
    g_windowHeight = (GLuint) h;
    int maxTextureImageUnits[2];
    int maxTextureSize[2];
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, maxTextureImageUnits);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, maxTextureSize);
    LOGV("%s: number of textures %d, size %d", __FUNCTION__, (int) maxTextureImageUnits[0],
         (int) maxTextureSize[0]);

    gProgram = createProgram(vertex_shader_yuv, frame_shader_yuv);

    if (!gProgram) {
        LOGV("Could not create program.");
        return -1;
    }

    glEnableVertexAttribArray(getHandle(gProgram, "aPosition"));
    glVertexAttribPointer(getHandle(gProgram, "aPosition"), VERTICES_DATA_POS_SIZE, GL_FLOAT, false, VERTICES_DATA_STRIDE_BYTES, g_vertices);
    glEnableVertexAttribArray(getHandle(gProgram, "aTextureCoord"));
    glVertexAttribPointer(getHandle(gProgram, "aTextureCoord"), VERTICES_DATA_UV_SIZE, GL_FLOAT, false, VERTICES_DATA_STRIDE_BYTES, &g_vertices[3]);

    glUseProgram(gProgram);

    glUniform1i(getHandle(gProgram, "Ytex"), 0); /* Bind Ytex to texture unit 0 */
    glUniform1i(getHandle(gProgram, "Utex"), 1); /* Bind Utex to texture unit 1 */
    glUniform1i(getHandle(gProgram, "Vtex"), 2); /* Bind Vtex to texture unit 2 */

    glViewport(0, 0, w, h);

    LOGV("YUV setup graphics ok");
    return 0;
}

static dt_av_frame_t g_frame;
static lock_t mutex;
static int frame_valid = 0;
static int g_inited = 0;

static int yuv_dttv_init() {
    lock_init(&mutex, NULL);
    memset(&g_frame, 0, sizeof(dt_av_frame_t));
    frame_valid = 0;
    g_inited = 1;

    // Fixme -
    g_windowWidth = g_windowHeight = 0;
    g_textureHeight = g_textureWidth = 0;

    version();
    return 0;
}

static int yuv_set_parameter(int cmd, unsigned long arg) {
    return 0;
}

static int yuv_update_frame(dt_av_frame_t *frame) {
    if (g_inited == 0) {
        if (frame->data[0]) {
            free(frame->data[0]);
        }
        LOGV("Not inited yet");
        return -1;
    }

    lock(&mutex);
    // check frame not displayed
    if (frame_valid == 1 && g_frame.data != NULL) {
        free(g_frame.data[0]);
    }
    memcpy(&g_frame, frame, sizeof(dt_av_frame_t));
    frame_valid = 1;
    unlock(&mutex);

    gl_notify();
    LOGV("yuv update frame\n");
    return 0;
}


static int yuv_renderFrame() {
    if (frame_valid != 1) {
        return -1;
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
    return 0;
}

gl_ops_t gl_ops_yuv = {
    .init = yuv_dttv_init,
    .setup = yuv_setupGraphics,
    .set_parameter = yuv_set_parameter,
    .update = yuv_update_frame,
    .render = yuv_renderFrame
};