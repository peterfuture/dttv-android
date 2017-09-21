#include <gl_render.h>
#include <gl_ops.h>
#include <dttv_jni_utils.h>
#include <dttv_jni_log.h>
#include <plugin/filter/gl_filter_saturation.h>

struct rgb_saturation_ctx {
    lock_t mutex;
    int texture_width;
    int texture_height;
    int window_width;
    int window_height;

    float saturation;

    GLuint program;
    GLuint texture[1];
    dt_av_frame_t frame;
    int frame_valid;
    int inited;
};

static rgb_saturation_ctx ctx;

static int FLOAT_SIZE_BYTES = 4;
static int VERTICES_DATA_POS_SIZE = 3;
static int VERTICES_DATA_UV_SIZE = 2;
static int VERTICES_DATA_STRIDE_BYTES = (VERTICES_DATA_POS_SIZE + VERTICES_DATA_UV_SIZE) * FLOAT_SIZE_BYTES;
static int VERTICES_DATA_POS_OFFSET = 0 * FLOAT_SIZE_BYTES;
static int VERTICES_DATA_UV_OFFSET = VERTICES_DATA_POS_OFFSET + VERTICES_DATA_POS_SIZE * FLOAT_SIZE_BYTES;

const static char indices[] = {
        0, 3, 2, // first triangle
        0, 2, 1  // second triangle
};

const static GLfloat vertices[20] = {
        // X, Y, Z, U, V
        -1, -1, 0, 0, 1, // Bottom Left
        1, -1, 0, 1, 1, //Bottom Right
        1, 1, 0, 1, 0, //Top Right
        -1, 1, 0, 0, 0 //Top Left
};

#define TAG "RGB-SATURATION-GL"

static int rgb_saturation_init() {
    ctx = {0};
    lock_init(&ctx.mutex, NULL);
    ctx.inited = 1;
    LOGV("rgb saturation init ok");
    return 0;
}

static int rgb_saturation_setup(int w, int h) {
    ctx.window_width = w;
    ctx.window_height = h;

    int maxTextureImageUnits[2];
    int maxTextureSize[2];
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, maxTextureImageUnits);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, maxTextureSize);
    LOGV("%s: number of textures %d, size %d", __FUNCTION__, (int) maxTextureImageUnits[0],
         (int) maxTextureSize[0]);

    ctx.program = createProgram(vertex_shader_saturation, frame_shader_saturation);

    glEnableVertexAttribArray(getHandle(ctx.program, "aPosition"));
    glVertexAttribPointer(getHandle(ctx.program, "aPosition"), VERTICES_DATA_POS_SIZE, GL_FLOAT, false, VERTICES_DATA_STRIDE_BYTES, vertices);
    glEnableVertexAttribArray(getHandle(ctx.program, "aTextureCoord"));
    glVertexAttribPointer(getHandle(ctx.program, "aTextureCoord"), VERTICES_DATA_UV_SIZE, GL_FLOAT, false, VERTICES_DATA_STRIDE_BYTES, &vertices[3]);

    glUseProgram(ctx.program);
    glUniform1i(getHandle(ctx.program, "sTexture"), 0);

    ctx.saturation = 0.0f;
    glUniform1f(getHandle(ctx.program, "saturation"), ctx.saturation);

    glViewport(0, 0, w, h);
    LOGV("rgb saturation setup ok. w:%d h:%d \n", w, h);
    return 0;
}

static int rgb_saturation_set_parameter(int cmd, unsigned long arg1, unsigned long arg2) {
    if (ctx.inited == 0) {
        return -1;
    }
    ctx.saturation = ((float)arg1/100);
    glUniform1f(getHandle(ctx.program, "saturation"), ctx.saturation);
    return 0;
}

static int rgb_saturation_update_frame(dt_av_frame_t *frame) {
    if (ctx.inited == 0) {
        if (frame->data[0]) {
            free(frame->data[0]);
        }
        LOGV("Not inited yet");
        return -1;
    }

    lock(&ctx.mutex);
    // check frame not displayed
    if (ctx.frame_valid == 1 && ctx.frame.data != NULL) {
        free(ctx.frame.data[0]);
    }
    memcpy(&ctx.frame, frame, sizeof(dt_av_frame_t));
    ctx.frame_valid = 1;
    unlock(&ctx.mutex);

    gl_notify();
    return 0;
}

static int setupTextures(uint8_t *data, GLsizei width, GLsizei height, int gen) {
    if(gen)
        glGenTextures(1, ctx.texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ctx.texture[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE,
                 (const GLvoid *) data);

    LOGV("setupTextures ok");
    return 0;
}

static int rgb_saturation_render() {

    if (ctx.frame_valid != 1) {
        LOGV("rgb saturation render failed. no valid frame.\n");
        return -1;
    }
    lock(&ctx.mutex);
    uint8_t *data = (uint8_t *) (ctx.frame.data[0]);
    int width = ctx.frame.width;
    int height = ctx.frame.height;

    glUseProgram(ctx.program);
    if (ctx.texture_width != ctx.window_width ||
        ctx.texture_height != ctx.window_height) {
        LOGV("TEXTREUE w:%d h:%d . [%d:%d] \n", (int) ctx.texture_width, (int) ctx.texture_height,
             (int) ctx.window_width, (int) ctx.window_height);
        setupTextures(data, width, height, 1);
        ctx.texture_width = ctx.window_width;
        ctx.texture_height = ctx.window_height;
    } else {
        setupTextures(data, width, height, 0);
    }
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
    checkGlError("glDrawArrays");
    free(data);
    ctx.frame_valid = 0;
    memset(&ctx.frame, 0, sizeof(dt_av_frame_t));
    unlock(&ctx.mutex);
    return 0;
}

gl_ops_t gl_ops_saturation = {
    .init = rgb_saturation_init,
    .setup = rgb_saturation_setup,
    .set_parameter = rgb_saturation_set_parameter,
    .update = rgb_saturation_update_frame,
    .render = rgb_saturation_render
};