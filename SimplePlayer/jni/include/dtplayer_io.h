#ifndef DTPLAYER_IO_H
#define DTPLAYER_IO_H

#include "dtplayer.h"

int start_io_thread (dtplayer_context_t * dtp_ctx);
int pause_io_thread (dtplayer_context_t * dtp_ctx);
int resume_io_thread (dtplayer_context_t * dtp_ctx);
int stop_io_thread (dtplayer_context_t * dtp_ctx);

#endif
