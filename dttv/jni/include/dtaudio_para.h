#ifndef DTAUDIO_PARA_H
#define DTAUDIO_PARA_H

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

#endif
