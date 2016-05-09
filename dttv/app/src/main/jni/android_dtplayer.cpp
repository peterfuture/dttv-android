// dtp_api.cpp
// TOP-LEVEL API provided for DtPlayer.java

#include "android_dtplayer.h"
#include "android_opengl.h"
#include "android_jni.h"

extern "C"{

#include <android/log.h>
#include <stdlib.h>
#include "dt_lock.h"

#include "native_log.h"

}
#define TAG "DTP-API"

extern "C" int dtap_change_effect(ao_wrapper_t *wrapper, int id);
#ifdef ENABLE_OPENSL
extern "C" void ao_opensl_setup(ao_wrapper_t *ao);
#endif
#ifdef ENABLE_AUDIOTRACK
extern "C" void ao_audiotrack_setup(ao_wrapper_t *ao);
#endif 
extern "C" void vo_android_setup(vo_wrapper_t *vo);
extern "C" void vd_stagefright_setup(vd_wrapper_t *vd);

namespace android {

DTPlayer::DTPlayer()
    :mListenner(NULL),
     status(0),
     mHWEnable(1),
     mDtpHandle(NULL),
     mCurrentPosition(-1),
     mSeekPosition(-1),
     mDuration(-1),
     mDisplayHeight(0),
     mDisplayWidth(0)
{
    memset(&media_info,0,sizeof(dt_media_info_t));
    dt_lock_init(&dtp_mutex, NULL);
    LOGV("DTPLAYER Constructor called \n");
}

DTPlayer::~DTPlayer()
{
    status = 0;
    mCurrentPosition = mSeekPosition = -1;
    mDtpHandle = NULL;
    if(mListenner)
        delete mListenner;
    gles2_release();
    LOGV( "DTPLAYER Destructor called \n");
}

int DTPlayer::setGLContext(void *p)
{
    mGLContext = p;
    return 0;
}

int DTPlayer::setListenner(dtpListenner *listenner)
{
    mListenner = listenner;
}

int DTPlayer::setDataSource(const char *file_name)
{
    int ret = 0;
    dt_media_info_t info;
    dtplayer_para_t para;
    para.disable_audio = para.disable_video = para.disable_sub = -1;
    para.height = para.width = -1;
    para.loop_mode = 0;
    para.audio_index = para.video_index = para.sub_index = -1;
    para.update_cb = NULL;
    para.disable_avsync = 0;
    
    memcpy(mUrl,file_name,strlen(file_name));
    mUrl[strlen(file_name)] = '\0';
	para.file_name = mUrl;
    para.cookie = this;
	para.update_cb = notify;
	//para.disable_audio=1;
	//para.disable_video=1;
	para.disable_sub=1;
    if(!mHWEnable)
        para.disable_hw_vcodec = 1;
	para.width = -1;
	para.height = -1;

    void *handle = mDtpHandle;
    if(handle != NULL)
    {
    	LOGV( "last player is running\n");
        goto FAILED;
    }

    //reset var
    DTPlayer::status = 0;
    DTPlayer::mCurrentPosition = -1;
    DTPlayer::mSeekPosition = -1;
    memset(&dtp_state,0,sizeof(player_state_t));

    handle = dtplayer_init(&para);
    if (!handle)
    {
    	LOGV("player init failed \n");
        goto FAILED;
    }
    //get media info
	ret = dtplayer_get_mediainfo(handle, &info);
	if(ret < 0)
	{
		LOGV("Get mediainfo failed, quit \n");
		return -1;
	}
    //update dtPlayer info with mediainfo

	memcpy(&media_info,&info,sizeof(dt_media_info_t));
    mDuration = info.duration;
    LOGV("Get Media Info Ok,filesize:%lld fulltime:%lld S \n",info.file_size,info.duration);
    
    mDtpHandle = handle;


    /*
     *
     *  register ext plugin
     *  AO - VO - VD
     *
     * */
#ifdef ENABLE_OPENSL
    ao_opensl_setup(&ao);
#endif
#ifdef ENABLE_AUDIOTRACK
    ao_audiotrack_setup(&ao);
#endif 
    dtplayer_register_ext_ao(&ao);
#ifdef ENABLE_ANDROID_OMX
    vd_stagefright_setup(&vd);
    dtplayer_register_ext_vd(&vd);
#endif
    vo_android_setup(&vo);
    dtplayer_register_ext_vo(&vo);

    status = PLAYER_INITED;
    return 0;

FAILED:
    mListenner->notify(MEDIA_ERROR);
    return -1;
}

int DTPlayer::prePare()
{
    if(status != PLAYER_INITED)
    {
        mListenner->notify(MEDIA_INVALID_CMD);
        return -1;
    }
    status = PLAYER_PREPARED;
    mListenner->notify(MEDIA_PREPARED);
    return 0;
}

int DTPlayer::prePareAsync()
{
    if(status != PLAYER_INITED)
    {
        mListenner->notify(MEDIA_INVALID_CMD);
        return -1;
    }
    status = PLAYER_PREPARED;
    mListenner->notify(MEDIA_PREPARED);
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
    	LOGV( "player is running \n");
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
    
    LOGV( "seekto %d s \n",pos);
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
        	LOGV( "seekTo execute \n");
            mSeekPosition = pos;
            dtplayer_seekto(handle,pos);
        }
        else
        	LOGV( "seekTo is ececuting \n");
    }
    else
    	LOGV("seekTo is not ececuting in status:%d \n",status);

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

    if(status >= PLAYER_STOPPED)
        goto END;

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

int DTPlayer::setVideoMode(int mode)
{
    vstream_info_t *vstream = NULL;
    void *handle = mDtpHandle;
    if(!handle)
        return -1;
    
    vstream = media_info.vstreams[0];

    int orig_width = vstream->width;
    int orig_height = vstream->height; 

    int dw, dh;
    //reset w h
    switch(mode)
    {
        case DT_SCREEN_MODE_NORMAL:
            dw = orig_width;
            dh = orig_height;
            break;
        case DT_SCREEN_MODE_FULL:
            dw = orig_width;
            dh = orig_height;
            break;
        case DT_SCREEN_MODE_16_9:
            dw = orig_width;
            dh = orig_height;
            break;
        case DT_SCREEN_MODE_4_3:
            dw = orig_width;
            dh = orig_height;
            break;
        default:
            return 0;
    }

    dtplayer_set_video_size(handle, dw, dh);
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
    LOGV( "getCurrentPos:%d status:%d \n", mCurrentPosition,status);
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

int DTPlayer::setAudioEffect(int id)
{
    void *handle = mDtpHandle;
    dt_lock(&dtp_mutex);
    if(!handle)
    {
        dt_unlock(&dtp_mutex);
        return 0;
    }
#ifdef ENABLE_DTAP
    dtap_change_effect(&ao, id);
#endif
    dt_unlock(&dtp_mutex);
    return 0;
}

int DTPlayer::setHWEnable(int enable)
{
    mHWEnable = (enable == 0)?0:1;    
    return 0;
}

int DTPlayer::Notify(int msg)
{
    mListenner->notify(msg);
    return 0;
}

int DTPlayer::notify(void *cookie, player_state_t *state)
{
    DTPlayer *dtp = (DTPlayer *)cookie;
    dt_lock(&dtp->dtp_mutex);
    int ret = 0;
    void *handle = dtp->mDtpHandle;
    if(dtp->status == PLAYER_STOPPED)
    {
        ret = -1;
        goto END;
    }
    memcpy(&dtp->dtp_state,state,sizeof(player_state_t));
	if (state->cur_status == PLAYER_STATUS_EXIT)
	{
		LOGV("PLAYER EXIT OK\n");
        dtp->mListenner->notify(MEDIA_PLAYBACK_COMPLETE);
        dtp->status = PLAYER_EXIT;
        goto END;
	}
	else if(state->cur_status == PLAYER_STATUS_SEEK_EXIT)
	{
		LOGV( "SEEK COMPLETE \n");
	    if(dtp->mCurrentPosition != dtp->mSeekPosition)
        {
            //still have seek request
            dtp->mSeekPosition = dtp->mCurrentPosition;
            dtplayer_seekto(handle,dtp->mCurrentPosition);
            LOGV("queued seek to %d \n",dtp->mCurrentPosition);
        }
        else
        {
            //last seek complete, return to running
            dtp->mSeekPosition = -1;
            LOGV( "seek complete !\n");
        }
        goto END;
	}
    
    if(dtp->status == PLAYER_SEEKING && state->cur_status == PLAYER_STATUS_RUNNING && state->last_status == PLAYER_STATUS_SEEK_EXIT)
    {
        if(dtp->mSeekPosition > 0) // receive seek again
            goto END;
        LOGV("set status to running from seek complete \n");
        dtp->status = PLAYER_RUNNING;
        dtp->mListenner->notify(MEDIA_INFO);
    }

    LOGV("UPDATECB CURSTATUS:%x status:%d \n", state->cur_status, dtp->status);
    LOGV("CUR TIME %lld S  FULL TIME:%lld  \n",state->cur_time,state->full_time);
END:
    dt_unlock(&dtp->dtp_mutex);
	return ret;
}

} // end namespace android
