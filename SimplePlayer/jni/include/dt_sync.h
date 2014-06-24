#ifndef DT_SYNC_H
#define DT_SYNC_H

typedef enum
{
    DT_SYNC_AUDIO_MASTER,       /* default choice */
    DT_SYNC_VIDEO_MASTER,
    DT_SYNC_EXTERNAL_CLOCK,     /* synchronize to an external clock */
} dt_sync_mode_t;

#endif
