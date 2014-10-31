#ifndef OPENGL_ANDROID_H
#define OPENGL_ANDROID_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "dt_av.h"
#include "dt_lock.h"
#include "android_dtplayer.h"

#define RGB565(r, g, b)  (((r) << (5+6)) | ((g) << 6) | (b))
#define GLRENDER_STATUS_IDLE 0
#define GLRENDER_STATUS_RUNNING 1
#define GLRENDER_STATUS_QUIT 2

namespace android{

enum {  
    ATTRIB_VERTEX,  
    ATTRIB_TEXTURE,  
}; 
typedef struct{
    int g_width;
    int g_height;

    GLuint g_texYId;
    GLuint g_texUId;
    GLuint g_texVId;
    GLuint simpleProgram;

    dt_av_frame_t frame;

	int status;
    int invalid_frame;
    int vertex_index;

    int initialized;

    DTPlayer *mp; // point to dtplayer
}gles2_ctx_t;

void gles2_setup();
void gles2_reg_player(DTPlayer *mp); 
void gles2_init(); 
void gles2_release();
int gles2_surface_changed(int w, int h);
int gles2_draw_frame();

}
#endif
