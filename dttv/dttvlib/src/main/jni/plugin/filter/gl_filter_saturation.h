#ifndef GL_FILTER_SATURATION
#define GL_FILTER_SATURATION

const char vertex_shader_saturation[] = {
        "attribute vec4 aPosition;\n"
        "attribute vec4 aTextureCoord;\n"
        "varying vec2 vTextureCoord;\n"

        "void main() {\n"
        "  gl_Position = aPosition;\n"
        "  vTextureCoord = aTextureCoord.xy;\n"
        "}\n"
};

static const char frame_shader_saturation[] = {
        "varying highp vec2 vTextureCoord;\n"
        "uniform sampler2D sTexture;\n"
        "uniform lowp float saturation = 0.0;\n"
        "const mediump vec3 luminanceWeighting = vec3(0.2125, 0.7154, 0.0721);\n"

        "void main() {\n"
        "   lowp vec4 textureColor = texture2D(sTexture, vTextureCoord);\n"
        "   lowp float luminance = dot(textureColor.rgb, luminanceWeighting);\n"
        "   lowp vec3 greyScaleColor = vec3(luminance);\n"
        "   gl_FragColor = vec4(mix(greyScaleColor, textureColor.rgb, saturation), textureColor.w);\n"
        "}\n"
};

#endif