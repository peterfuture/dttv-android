#ifndef DTP_NATIVE_API_H
#define DTP_NATIVE_API_H

#include "android_dtplayer.h"
extern "C"{
#include "dtplayer_api.h"
}
namespace android {

const static int MEDIA_PREPARED = 1;
const static int MEDIA_PLAYBACK_COMPLETE = 2;
const static int MEDIA_BUFFERING_UPDATE = 3;
const static int MEDIA_SEEK_COMPLETE = 4;
const static int MEDIA_SET_VIDEO_SIZE = 5;
const static int MEDIA_FRESH_VIDEO = 99; // ugly code
const static int MEDIA_ERROR = 100;
const static int MEDIA_INVALID_CMD = 101;
const static int MEDIA_INFO = 200;
const static int MEDIA_CACHE = 300;
const static int MEDIA_HW_ERROR = 400;
const static int MEDIA_TIMED_TEXT = 1000;
const static int MEDIA_CACHING_UPDATE = 2000;

class DTPlayer{
public:
    DTPlayer();
    ~DTPlayer();
    int setListenner(dtpListenner *listenner);
    int setDataSource(const char * uri);
    int prePare();
    int prePareAsync();
    int start();
    int pause();
    int seekTo(int pos);
    int stop();
    int release();
    int reset();
    int setVideoMode(int mode);
    int setVideoSize(int w, int h);
    int getVideoHeight();
    int getVideoWidth();
    int isPlaying();
    int isQuitOK();
    int getCurrentPosition();
    int getDuration();
    int setAudioEffect(int id);
    int setHWEnable(int enable);
    static int notify(void *cookie, player_state_t *state);
private:

    enum {
        PLAYER_IDLE                = 0X0,
        PLAYER_INITED              = 0x01,
        PLAYER_PREPARED            = 0x02,
        PLAYER_PAUSED              = 0x04,
        PLAYER_RUNNING             = 0x08,
        PLAYER_SEEKING             = 0x10,
        PLAYER_STOPPED             = 0x20,
        PLAYER_EXIT                = 0x40,
        PLAYER_EOS                 = 0x200,
    };
    
    dt_media_info_t media_info;

    int status;
    void *mDtpHandle;
    int mDisplayWidth;
    int mDisplayHeight;
    char mUrl[2048];
    int mCurrentPosition;
    int mSeekPosition;
    int mDuration;
    dtpListenner *mListenner;
    dt_lock_t dtp_mutex;
    player_state_t dtp_state;
    int mHWEnable;
    int volume;
    ao_wrapper_t ao;
    vo_wrapper_t vo;
    vd_wrapper_t vd;
    int audio_pp_id;   // audio effect id
};

}

#endif
