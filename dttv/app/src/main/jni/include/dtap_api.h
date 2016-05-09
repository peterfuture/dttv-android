#ifndef dtap_API_H
#define dtap_API_H

#define MAX_NAME_LEN 1024

typedef enum{
    DTAP_ID_LVM     = 0x0,
    DTAP_ID_PRIVATE = 0x1
}dtap_id_t;

typedef enum{
    DTAP_EFFECT_EQ       = 0x0,
    DTAP_EFFECT_REVERB   = 0x1,
    
    DTAP_EFFECT_MAX      = 0x9,
}dtap_type_t;

typedef enum{
    //EQ PART
    EQ_EFFECT_NORMAL     = 0x00,
    EQ_EFFECT_CLASSICAL  = 0x01,
    EQ_EFFECT_DANCE      = 0x02,
    EQ_EFFECT_FLAT       = 0x03,
    EQ_EFFECT_FOLK       = 0x04,
    EQ_EFFECT_HEAVYMETAL = 0x05,
    EQ_EFFECT_HIPHOP     = 0x06,
    EQ_EFFECT_JAZZ       = 0x07,
    EQ_EFFECT_POP        = 0x08,
    EQ_EFFECT_ROCK       = 0x09,

    // REVERB
    REVERB_EFFECT_NONE   = 0x10,

    EQ_EFFECT_MAX        = 0x80
}dtap_item_t;

struct dtap_context;

typedef struct{
    uint8_t *in;
    int in_size;
}dtap_frame_t;

typedef struct{
    int samplerate;
    int channels;
    int data_width;
    dtap_type_t type;
    dtap_item_t item;
}dtap_para_t;

typedef struct{
    int id;
    int type;
    char *name;

    int (*init)     (struct dtap_context *ctx);
    int (*process)  (struct dtap_context *ctx, dtap_frame_t *frame);
    int (*config)   (struct dtap_context *ctx);
    int (*release)  (struct dtap_context *ctx);
}ap_wrapper_t;


/*
 * dtap_context_t 
 * every obj means one process context
 * multimle case support
 *
 * */
typedef struct dtap_context{
    dtap_para_t para;   // used to config ap
    char name[MAX_NAME_LEN];
    ap_wrapper_t *wrapper;
    uint8_t *out;
    int out_size;
    int inited;
    void *ap_priv;
}dtap_context_t;

int dtap_init(dtap_context_t *ctx);
int dtap_process(dtap_context_t *ctx, dtap_frame_t *frame);
int dtap_update(dtap_context_t *ctx);
int dtap_release(dtap_context_t *ctx);

#endif
