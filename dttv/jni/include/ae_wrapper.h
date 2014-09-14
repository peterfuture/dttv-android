#ifndef AE_WRAPPER_H

struct dtaudio_effect;

enum{
    DT_AE_NORMAL         = 0x0,
    DT_AE_CLASSICAL      = 0x1,
    DT_AE_DANCE          = 0x2,
    DT_AE_FLAT           = 0x4,
    DT_AE_FOLK           = 0x8,
    DT_AE_HEAVY_METAL    = 0x10,
    DT_AE_HIPPOP         = 0x20,
    DT_AE_JAZZ           = 0x40,
    DT_AE_POP            = 0x80,
    DT_AE_ROCK           = 0x100,
    DT_AE_MAX            = 0x100000
};

enum{
    AE_STATUS_ERR = -1,
    AE_STATUS_IDLE = 0,
    AE_STATUS_RUNNING,
};

typedef struct{
    int (*capacity) ();
    int (*init) (struct dtaudio_effect *ae, int ae_id);
    int (*reset) (struct dtaudio_effect *ae, int ae_id);
    int (*process) (struct dtaudio_effect *ae, dt_av_frame_t *frame);
    int (*release) (struct dtaudio_effect *ae);
    
    char *name;
    int type;
    int status;
    struct ae_wrapper *next;
}ae_wrapper_t;

typedef struct dtaudio_effect{
    dtaudio_para_t *para;
    ae_wrapper_t *wrapper;
    void *ae_priv;
}dtaudio_effect_t;

#endif
