//audio module
#ifndef DTAUDIO_ANDROID_H
#define DTAUDIO_ANDROID_H

#include <stdint.h>
#define AUDIO_EXTRADATA_SIZE 4096
typedef struct
{
    int channels,dst_channels;
    int samplerate,dst_samplerate;
    int data_width;
    int bps;
    int num, den;
    int extradata_size;
    unsigned char extradata[AUDIO_EXTRADATA_SIZE];
    int afmt;
    int audio_filter;
    int audio_output;
    void *avctx_priv;           //point to avcodec_context
} dtaudio_para_t;

typedef struct ao_wrapper
{
    int id;
    char *name;
    dtaudio_para_t para;

    int (*ao_init) (dtaudio_para_t *para);
    int (*ao_start) ();
    int (*ao_pause) ();
    int (*ao_resume) ();
    int (*ao_stop) ();
    int64_t (*ao_latency) ();
    int (*ao_level) ();
    int (*ao_write) (uint8_t * buf, int size);
    struct ao_wrapper *next;
    void *ao_priv;
} ao_wrapper_t;


#endif
