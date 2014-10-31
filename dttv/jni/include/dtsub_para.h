#ifndef DTSUB_PARA_H
#define DTSUB_PARA_H

#define SUB_EXTRADATA_SIZE 4096

typedef struct
{
    int sfmt;
    int width;
    int height;
    int sub_output;
    void *avctx_priv;
}dtsub_para_t;

#endif
