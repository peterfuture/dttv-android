#ifndef DTP_PLUGIN_H
#define DTP_PLUGIN_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "dtp_audio_plugin.h"
#include "dtp_video_plugin.h"
#include "dtp_sub_plugin.h"

typedef enum {
    DTP_PLUGIN_TYPE_AO = 0,
    DTP_PLUGIN_TYPE_AD,
    DTP_PLUGIN_TYPE_VO,
    DTP_PLUGIN_TYPE_VD,
    DTP_PLUGIN_TYPE_SD,
    DTP_PLUGIN_TYPE_SO,
    DTP_PLUGIN_TYPE_VF,
    DTP_PLUGIN_TYPE_STREAM,
    DTP_PLUGIN_TYPE_DEMUXER,
    DTP_PLUGIN_TYPE_MAX,
}
dtp_plugin_type_t;


#ifdef  __cplusplus
}
#endif

#endif
