#ifndef GL_FILTER_BLUR
#define GL_FILTER_BLUR

const char vertex_shader_rgb[] = {
        "attribute vec4 aPosition;\n"
        "attribute vec4 aTextureCoord;\n"
        "varying vec2 vTextureCoord;\n"

        "void main() {\n"
        "  gl_Position = aPosition;\n"
        "  vTextureCoord = aTextureCoord.xy;\n"
        "}\n"
};

static const char frame_shader_rgb[] = {
        "precision mediump float;\n"
        "varying highp vec2 vTextureCoord;\n"
        "uniform lowp sampler2D sTexture;\n"
        "void main() {\n"
        "   gl_FragColor = texture2D(sTexture, vTextureCoord);\n"
        "}\n"
};

#endif