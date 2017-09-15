#ifndef GL_FILTER_NONE
#define GL_FILTER_NONE

const char vertex_shader_none[] = {
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
static const char frame_shader_none[] = {
        "precision mediump float;\n"
        "uniform sampler2D Ytex, Utex, Vtex;\n"
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

#endif