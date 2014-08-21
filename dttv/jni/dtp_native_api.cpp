// dtp_api.cpp
// TOP-LEVEL API provided for DtPlayer.java

#include "android_dtplayer.h"
#include "dtp_native_api.h"

extern "C"{
#include <android/log.h>
#include <stdlib.h>

#include <GLES/gl.h>
#include <GLES/glext.h>

#include "render_android.h"
#include "dt_lock.h"

#define DEBUG_TAG "DTP-API"
}

extern int Notify(int status);

namespace android {

static player_state_t dtp_state;

int DTPlayer::status = 0;
int DTPlayer::mCurrentPosition = -1;
int DTPlayer::mSeekPosition = -1;
void *DTPlayer::mDtpHandle = NULL;
static dt_lock_t dtp_mutex;

DTPlayer::DTPlayer()
    :mListenner(NULL),
     mDuration(-1),
     mDisplayHeight(0),
     mDisplayWidth(0)
{
    memset(&media_info,0,sizeof(dt_media_info_t));
    dt_lock_init(&dtp_mutex, NULL);
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "DTPLAYER Constructor called \n");
}

DTPlayer::~DTPlayer()
{
    status = 0;
    mCurrentPosition = mSeekPosition = -1;
    mDtpHandle = NULL;
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "DTPLAYER Destructor called \n");
}

int DTPlayer::setListenner(dtpListenner *listenner)
{
    mListenner = listenner;
}

int DTPlayer::setDataSource(const char *file_name)
{
    render_init();
    dtplayer_para_t para;
    para.no_audio = para.no_video = para.no_sub = -1;
	para.height = para.width = -1;
	para.loop_mode = 0;
	para.audio_index = para.video_index = para.sub_index = -1;
	para.update_cb = NULL;
	para.sync_enable = -1;
    
    memcpy(mUrl,file_name,strlen(file_name));
    mUrl[strlen(file_name)] = '\0';
	para.file_name = mUrl;
	para.update_cb = updatePlayerState;
	//para.no_audio=1;
	//para.no_video=1;
	para.width = -1;
	para.height = -1;

    void *handle = mDtpHandle;
    if(handle != NULL)
    {
	    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "last player is running\n");
        return -1;
    }

    handle = dtplayer_init(&para);
    if (!handle)
    {
	    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "player init failed \n");
        return -1;
    }
    //get media info
	dt_media_info_t info;
	int ret = dtplayer_get_mediainfo(handle, &info);
	if(ret < 0)
	{
	    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Get mediainfo failed, quit \n");
		return -1;
	}
    //update dtPlayer info with mediainfo

	memcpy(&media_info,&info,sizeof(dt_media_info_t));
    mDuration = info.duration;
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Get Media Info Ok,filesize:%lld fulltime:%lld S \n",info.file_size,info.duration);
    
    mDtpHandle = handle;
    status = PLAYER_INITED;
    return 0;
}

int DTPlayer::prePare()
{
    if(status != PLAYER_INITED)
    {
        Notify(MEDIA_INVALID_CMD);
        return -1;
    }
    status = PLAYER_PREPARED;
    //mListenner->notify(MEDIA_PREPARED);
    Notify(MEDIA_PREPARED);
    return 0;
}

int DTPlayer::prePareAsync()
{
    if(status != PLAYER_INITED)
    {
        Notify(MEDIA_INVALID_CMD);
        return -1;
    }
    status = PLAYER_PREPARED;
    //mListenner->notify(MEDIA_PREPARED);
    Notify(MEDIA_PREPARED);
    return 0;
}

int DTPlayer::start()
{
    int ret = 0;
    void *handle = mDtpHandle;

    if(status < PLAYER_PREPARED)
    {
        ret = -1;
        goto END;
    }

    if(!handle)
    {
        ret = -1;
        goto END;
    }
    if(status == PLAYER_PAUSED) // maybe resume using start cmd
    {
        return pause();
    }

    if(status != PLAYER_PREPARED)
    {
        __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "player is running \n");
        goto END;
    }

    ret = dtplayer_start(handle);
    if(ret < 0)
    {
        ret = -1;
        goto END;
    }
    status = PLAYER_RUNNING;

END:
    return ret;
}

int DTPlayer::pause()
{
    void *handle = mDtpHandle;
    dt_lock(&dtp_mutex);
    if(!handle)
    {
        dt_unlock(&dtp_mutex);
        return -1;
    }
    if(status == PLAYER_RUNNING)
    {
        dtplayer_pause(handle);
        status = PLAYER_PAUSED;
    }
    else if(status == PLAYER_PAUSED)
    {
        dtplayer_resume(handle);
        status = PLAYER_RUNNING;
    }

    dt_unlock(&dtp_mutex);
    return 0;
}

int DTPlayer::seekTo(int pos) // ms
{
    void *handle = mDtpHandle;
    int ret = 0;
    
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "seekto %d s \n",pos);
    dt_lock(&dtp_mutex);    
    if(!handle)
    {
        ret = -1;
        goto END;
    }

    if(status == PLAYER_RUNNING || status == PLAYER_PAUSED ||  status == PLAYER_SEEKING)
    {
        if(pos < 0)
        {
            ret = -1;
            goto END;
        }
        if(pos >= mDuration)
        {
            pos = mDuration;
        }
        mCurrentPosition = pos;
        status = PLAYER_SEEKING;
        if(mSeekPosition < 0)
        {
            __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "seekTo execute \n");
            mSeekPosition = pos;
            dtplayer_seekto(handle,pos);
        }
        else
            __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "seekTo is ececuting \n");
    }
    else
        __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "seekTo is not ececuting in status:%d \n",status);

END:
    dt_unlock(&dtp_mutex);
    return 0; 
}

int DTPlayer::stop()
{
    int ret = 0;
    void *handle = mDtpHandle;
    if(!handle)
    {
        ret = -1;
        goto END;
    }
    if(status <= PLAYER_PREPARED)
    {
        ret = -1;
        goto END;
    }
    ret = dtplayer_stop(handle);
    mDtpHandle = NULL;
    status = PLAYER_STOPPED;
END:
    return ret;
}

int DTPlayer::reset()
{
    //do nothing
    return 0;
}

int DTPlayer::setVideoSize(int w, int h)
{
    void *handle = mDtpHandle;
    if(!handle)
        return -1;
    dtplayer_set_video_size(handle, w, h);
    return 0;
}

int DTPlayer::getVideoWidth()
{
    void *handle = mDtpHandle;
    vstream_info_t *vstream = NULL;
    if(!handle)
        return -1;
    if (media_info.vst_num == 0)
        return -1;
    vstream = media_info.vstreams[0];
    return vstream->width;
}

int DTPlayer::getVideoHeight()
{
    void *handle = mDtpHandle;
    vstream_info_t *vstream = NULL;
    if(!handle)
        return -1;
    if (media_info.vst_num == 0)
        return -1;
    vstream = media_info.vstreams[0];
    return vstream->height;
}

int DTPlayer::isPlaying()
{
    void *handle = mDtpHandle;
    int isPlaying = 1;
    dt_lock(&dtp_mutex);
    if(!handle)
    {
        dt_unlock(&dtp_mutex);
        return -1;
    }
    isPlaying =  (status == PLAYER_RUNNING);
    dt_unlock(&dtp_mutex);
    return isPlaying;
}

int DTPlayer::isQuitOK()
{
    void *handle = mDtpHandle;
    int isQuitOK = 0;
    dt_lock(&dtp_mutex);
    if(!handle)
    {
        dt_unlock(&dtp_mutex);
        return 1;
    }
    isQuitOK = (status == PLAYER_EXIT);
    dt_unlock(&dtp_mutex);
    return isQuitOK;
}

int DTPlayer::getCurrentPosition()
{
    void *handle = mDtpHandle;
    int cur_pos = -1;
    dt_lock(&dtp_mutex);
    if(!handle)
    {
        dt_unlock(&dtp_mutex);
        return 0;
    }
    if(status == PLAYER_RUNNING)
    {
        mCurrentPosition = dtp_state.cur_time;
    }

    cur_pos = mCurrentPosition;
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "getCurrentPos:%d status:%d \n", mCurrentPosition,status);
    dt_unlock(&dtp_mutex);
    return cur_pos;
}

int DTPlayer::getDuration()
{
    void *handle = mDtpHandle;
    dt_lock(&dtp_mutex);
    if(!handle)
    {
        dt_unlock(&dtp_mutex);
        return 0;
    }
    dt_unlock(&dtp_mutex);
    return mDuration;

}

int DTPlayer::updatePlayerState(player_state_t *state)
{
    void *handle = mDtpHandle;
    int ret = 0;
    dt_lock(&dtp_mutex);
    if(status == PLAYER_STOPPED)
    {
        ret = -1;
        goto END;
    }
    memcpy(&dtp_state,state,sizeof(player_state_t));
	if (state->cur_status == PLAYER_STATUS_EXIT)
	{
		__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "PLAYER EXIT OK\n");
		Notify(MEDIA_PLAYBACK_COMPLETE);
        status = PLAYER_EXIT;
        goto END;
	}
	else if(state->cur_status == PLAYER_STATUS_SEEK_EXIT)
	{
	    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "SEEK COMPLETE \n");
	    if(mCurrentPosition != mSeekPosition)
        {
            //still have seek request
            mSeekPosition = mCurrentPosition;
            dtplayer_seekto(handle,mCurrentPosition);
	        __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "queued seek to %d \n",mCurrentPosition);
        }
        else
        {
            //last seek complete, return to running
            mSeekPosition = -1;
	        __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "seek complete !\n");
        }
        Notify(MEDIA_SEEK_COMPLETE);
        goto END;
	}
    
    if(status == PLAYER_SEEKING && state->cur_status == PLAYER_STATUS_RUNNING && state->last_status == PLAYER_STATUS_SEEK_EXIT)
    {
        if(mSeekPosition > 0) // receive seek again
            goto END;
	    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "set status to running from seek complete \n");
        status = PLAYER_RUNNING;
        Notify(MEDIA_INFO);
    }

	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "UPDATECB CURSTATUS:%x status:%d \n", state->cur_status,status);
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "CUR TIME %lld S  FULL TIME:%lld  \n",state->cur_time,state->full_time);
END:
    dt_unlock(&dtp_mutex);
	return ret;
}

} // end namespace android
