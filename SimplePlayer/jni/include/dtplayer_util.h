#ifndef DTPLAYER_UTIL_H
#define DTPLAYER_UTIL_H

#include "dtplayer.h"

int player_host_init (dtplayer_context_t * dtp_ctx);
int player_host_start (dtplayer_context_t * dtp_ctx);
int player_host_pause (dtplayer_context_t * dtp_ctx);
int player_host_resume (dtplayer_context_t * dtp_ctx);
int player_host_stop (dtplayer_context_t * dtp_ctx);

#endif
