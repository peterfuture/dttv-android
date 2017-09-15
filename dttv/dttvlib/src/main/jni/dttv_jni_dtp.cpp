// dtp_api.cpp
// TOP-LEVEL API provided for DtPlayer.java

#include <dtp_state.h>
#include <dtp_audio_plugin.h>
#include <dtp_video_plugin.h>
#include <gl_render_yuv.h>
#include "dttv_jni_dtp.h"

#define TAG "NATIVE-DTP"

extern vo_wrapper_t vo_android_opengl;
extern vo_wrapper_t vo_android_surface;
extern ao_wrapper_t ao_opensl_ops;
extern so_wrapper_t so_android_ops;

namespace android {

    DTPlayer::DTPlayer()
            : mListenner(NULL),
              status(0),
              mHWEnable(1),
              mDtpHandle(NULL),
              mCurrentPosition(-1),
              mSeekPosition(-1),
              mDuration(-1),
              mDisplayHeight(0),
              mDisplayWidth(0) {
        memset(&media_info, 0, sizeof(dtp_media_info_t));
        lock_init(&dtp_mutex, NULL);
        mNativeWindow = -1;
        mSurface = -1;
        mRenderType = 0;
        LOGV("dtplayer constructor ok \n");
    }

    DTPlayer::DTPlayer(dttvListenner *listenner)
            : status(0),
              mHWEnable(1),
              mDtpHandle(NULL),
              mCurrentPosition(-1),
              mSeekPosition(-1),
              mDuration(-1),
              mDisplayHeight(0),
              mDisplayWidth(0) {
        memset(&media_info, 0, sizeof(dtp_media_info_t));
        lock_init(&dtp_mutex, NULL);
        mListenner = listenner;
        mNativeWindow = -1;
        mSurface = -1;
        mRenderType = 0;
        LOGV("dtplayer constructor ok \n");
    }

    DTPlayer::~DTPlayer() {
        status = 0;
        mCurrentPosition = mSeekPosition = -1;
        mDtpHandle = NULL;
        if (mListenner) {
            delete mListenner;
        }
        LOGV("dtplayer destructor called \n");
    }

    int DTPlayer::setGLContext(void *p) {
        mGLContext = p;
        return 0;
    }

    int DTPlayer::setListenner(dttvListenner *listenner) {
        LOGV("[%d:%p] [%d:%p]", sizeof(mListenner), this->mListenner, sizeof(listenner), listenner);
        this->mListenner = listenner;
        return 0;
    }

    // Remove Later - Never used
    void DTPlayer::setNativeWindow(ANativeWindow *window) {
        mNativeWindow = (unsigned long) window;
        mRenderType = 0;
    }

    void DTPlayer::setSurface(void *surface) {
        mSurface = (unsigned long) surface;
        mRenderType = 0;
    }

    void DTPlayer::setGLSurfaceView() {
        LOGV("Use GLSurfaceView. Register gl render \n");
        mRenderType = 1;
    }

    int DTPlayer::supportMediaCodec() {
        dtp_media_info_t info;
        int ret = 0;
        dtvideo_format_t vfmt;
        dtaudio_format_t afmt;

        ret = dtplayer_get_mediainfo(mDtpHandle, &info);
        if (ret < 0) {
            LOGV("Get mediainfo failed, quit \n");
            return 0;
        }

        if (info.has_video == 0)
            return 0;
        vstream_info_t *stream = info.tracks.vstreams[info.cur_vst_index];
        vfmt = stream->format;
        if (vfmt == DT_VIDEO_FORMAT_H264 || vfmt == DT_VIDEO_FORMAT_HEVC)
            return 1;

        return 0;
    }

    void DTPlayer::setupRender() {
        dtp_media_info_t info;
        int ret = 0;
        dtaudio_format_t afmt;
        ret = dtplayer_get_mediainfo(mDtpHandle, &info);
        if (ret < 0) {
            LOGV("Get mediainfo failed, quit \n");
            return;
        }

        // audio render setup
        if (info.has_audio) {
            astream_info_t *stream = info.tracks.astreams[info.cur_ast_index];
            afmt = stream->format;
            LOGI("current afmt: %d \n", afmt);
            dtplayer_register_plugin(DTP_PLUGIN_TYPE_AO, &ao_opensl_ops);
        }

        // video render setup
        if (info.has_video) {
            if (mRenderType == 0) {
                dtplayer_register_plugin(DTP_PLUGIN_TYPE_VO, &vo_android_surface);
                dtplayer_set_parameter(mDtpHandle, DTP_CMD_SET_VODEVICE, mSurface);
            } else if (mRenderType == 1) {
                dtplayer_register_plugin(DTP_PLUGIN_TYPE_VO, &vo_android_opengl);
            }

            LOGI("setup render. use %s.\n", (mRenderType == 0) ? "surfaceview" : "opengl");
        }

        // sub render setup
        if(info.has_sub) {
            dtplayer_register_plugin(DTP_PLUGIN_TYPE_SO, &so_android_ops);
            LOGI("register android vo.\n");
        }

        return;
    }

    int DTPlayer::setDataSource(const char *file_name) {
        int ret = 0;
        dtp_media_info_t info;
        dtplayer_para_t para;
        memset(&para, 0, sizeof(dtplayer_para_t));
        para.disable_audio = para.disable_video = para.disable_sub = -1;
        para.height = para.width = -1;
        para.loop_mode = 1;
        para.audio_index = para.video_index = para.sub_index = -1;
        para.update_cb = NULL;
        para.disable_avsync = 0;

        memcpy(mUrl, file_name, strlen(file_name));
        mUrl[strlen(file_name)] = '\0';
        para.file_name = mUrl;
        para.cookie = this;
        para.update_cb = notify;
        //para.disable_audio=1;
        //para.disable_video=1;
        //para.disable_sub = 1;
        if (!mHWEnable) {
            para.disable_hw_vcodec = 1;
            LOGV("disable hw codec\n");
        }
        para.width = -1;
        para.height = -1;

        void *handle = mDtpHandle;
        if (handle != NULL) {
            LOGV("last player is running\n");
            goto FAILED;
        }

        // default avoptions
        // dtplayer_set_option(NULL, 0, "protocol_whitelist", "file,http,hls,udp,rtp,rtsp,tcp");

        //reset var
        DTPlayer::status = 0;
        DTPlayer::mCurrentPosition = -1;
        DTPlayer::mSeekPosition = -1;
        memset(&dtp_state, 0, sizeof(dtp_state_t));

        handle = dtplayer_init(&para);
        if (!handle) {
            LOGV("player init failed \n");
            goto FAILED;
        }
        //get media info
        ret = dtplayer_get_mediainfo(handle, &info);
        if (ret < 0) {
            LOGV("Get mediainfo failed, quit \n");
            return -1;
        }
        //update dtPlayer info with mediainfo

        memcpy(&media_info, &info, sizeof(dtp_media_info_t));
        mDuration = info.duration;
        mDtpHandle = handle;
        LOGV("Get Media Info Ok,filesize:%lld fulltime:%lld S \n", info.file_size, info.duration);

        status = PLAYER_INITED;
        return 0;

        FAILED:
        mListenner->notify(MEDIA_ERROR);
        return -1;
    }

    int DTPlayer::prePare() {
        if (status != PLAYER_INITED) {
            mListenner->notify(MEDIA_INVALID_CMD);
            return -1;
        }
        status = PLAYER_PREPARED;
        mListenner->notify(MEDIA_PREPARED);
        return 0;
    }

    int DTPlayer::prePareAsync() {
        if (status != PLAYER_INITED) {
            mListenner->notify(MEDIA_INVALID_CMD);
            return -1;
        }
        status = PLAYER_PREPARED;
        mListenner->notify(MEDIA_PREPARED);
        return 0;
    }

    int DTPlayer::start() {
        int ret = 0;
        void *handle = mDtpHandle;

        if (status < PLAYER_PREPARED) {
            ret = -1;
            goto END;
        }

        if (!handle) {
            ret = -1;
            goto END;
        }
        if (status == PLAYER_PAUSED) { // maybe resume using start cmd
            return pause();
        }

        if (status != PLAYER_PREPARED) {
            LOGV("player is running \n");
            goto END;
        }

        setupRender();

        ret = dtplayer_start(handle);
        if (ret < 0) {
            ret = -1;
            goto END;
        }
        status = PLAYER_RUNNING;

        END:
        return ret;
    }

    int DTPlayer::pause() {
        void *handle = mDtpHandle;
        lock(&dtp_mutex);
        if (!handle) {
            unlock(&dtp_mutex);
            return -1;
        }
        if (status == PLAYER_RUNNING) {
            dtplayer_pause(handle);
            status = PLAYER_PAUSED;
        } else if (status == PLAYER_PAUSED) {
            dtplayer_resume(handle);
            status = PLAYER_RUNNING;
        }

        unlock(&dtp_mutex);
        return 0;
    }

    int DTPlayer::seekTo(int pos) // ms
    {
        void *handle = mDtpHandle;
        int ret = 0;

        LOGV("seekto %d s \n", pos);
        lock(&dtp_mutex);
        if (!handle) {
            ret = -1;
            goto END;
        }

        if (status == PLAYER_RUNNING || status == PLAYER_PAUSED || status == PLAYER_SEEKING) {
            if (pos < 0) {
                ret = -1;
                goto END;
            }
            if (pos >= mDuration) {
                pos = mDuration;
            }
            mCurrentPosition = pos;
            status = PLAYER_SEEKING;
            if (mSeekPosition < 0) {
                LOGV("seekTo execute \n");
                mSeekPosition = pos;
                dtplayer_seekto(handle, pos);
            } else {
                LOGV("seekTo is ececuting \n");
            }
        } else {
            LOGV("seekTo is not ececuting in status:%d \n", status);
        }

        END:
        unlock(&dtp_mutex);
        return 0;
    }

    int DTPlayer::stop() {
        int ret = 0;
        void *handle = mDtpHandle;
        if (!handle) {
            ret = -1;
            goto END;
        }
        if (status <= PLAYER_PREPARED) {
            ret = -1;
            goto END;
        }

        if (status >= PLAYER_STOPPED) {
            goto END;
        }

        status = PLAYER_STOPPED;
        ret = dtplayer_stop(handle);
        //mDtpHandle = NULL;

        END:
        return ret;
    }

    int DTPlayer::reset() {
        //do nothing
        return 0;
    }

    int DTPlayer::setVideoMode(int mode) {
        vstream_info_t *vstream = NULL;
        void *handle = mDtpHandle;
        if (!handle) {
            return -1;
        }

        vstream = media_info.tracks.vstreams[0];

        int orig_width = vstream->width;
        int orig_height = vstream->height;

        int dw, dh;
        //reset w h
        switch (mode) {
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

        //dtplayer_set_video_size(handle, dw, dh);
        return 0;
    }

    int DTPlayer::setVideoSize(int w, int h) {
        void *handle = mDtpHandle;
        if (!handle) {
            return -1;
        }
        //dtplayer_set_video_size(handle, w, h);
        return 0;
    }

    int DTPlayer::getVideoWidth() {
        void *handle = mDtpHandle;
        vstream_info_t *vstream = NULL;
        if (!handle) {
            return -1;
        }
        if (media_info.tracks.vst_num == 0) {
            return -1;
        }
        vstream = media_info.tracks.vstreams[0];
        return vstream->width;
    }

    int DTPlayer::getVideoHeight() {
        void *handle = mDtpHandle;
        vstream_info_t *vstream = NULL;
        if (!handle) {
            return -1;
        }
        if (media_info.tracks.vst_num == 0) {
            return -1;
        }
        vstream = media_info.tracks.vstreams[0];
        return vstream->height;
    }

    dtp_media_info_t *DTPlayer::getMediaInfo() {
        return &media_info;
    }

    int DTPlayer::isPlaying() {
        void *handle = mDtpHandle;
        int isPlaying = 1;
        lock(&dtp_mutex);
        if (!handle) {
            unlock(&dtp_mutex);
            return -1;
        }
        isPlaying = (status == PLAYER_RUNNING);
        unlock(&dtp_mutex);
        return isPlaying;
    }

    int DTPlayer::isQuitOK() {
        void *handle = mDtpHandle;
        int isQuitOK = 0;
        lock(&dtp_mutex);
        if (!handle) {
            unlock(&dtp_mutex);
            return 1;
        }
        isQuitOK = (status == PLAYER_EXIT);
        unlock(&dtp_mutex);
        return isQuitOK;
    }

    int DTPlayer::getCurrentPosition() {
        return mCurrentPosition;
    }

    int DTPlayer::getDuration() {
        void *handle = mDtpHandle;
        lock(&dtp_mutex);
        if (!handle) {
            unlock(&dtp_mutex);
            return 0;
        }
        unlock(&dtp_mutex);
        return mDuration;

    }

    int DTPlayer::setAudioEffect(int id) {
        void *handle = mDtpHandle;
        lock(&dtp_mutex);
        if (!handle) {
            unlock(&dtp_mutex);
            return 0;
        }
#ifdef ENABLE_DTAP
            dtap_change_effect(&ao, id);
#endif
        unlock(&dtp_mutex);
        return 0;
    }

    int DTPlayer::setHWEnable(int enable) {
        mHWEnable = (enable == 0) ? 0 : 1;
        return 0;
    }

    int DTPlayer::Notify(int msg) {
        mListenner->notify(msg);
        return 0;
    }

    int DTPlayer::notify(void *cookie, dtp_state_t *state) {
        DTPlayer *dtp = (DTPlayer *) cookie;
        lock(&dtp->dtp_mutex);
        int ret = 0;
        void *handle = dtp->mDtpHandle;

        // Handle Error
        if (state->cur_status == PLAYER_STATUS_ERROR) {
            dtp->status = PLAYER_STOPPED;
            dtp->mListenner->notify(MEDIA_ERROR);
            LOGV("Error \n");
            ret = -1;
            goto END;
        }

        if (dtp->status == PLAYER_STOPPED) {
            ret = -1;
            goto END;
        }
        if (dtp->status == PLAYER_RUNNING) {
            dtp->mCurrentPosition = state->cur_time;
        }

        // mediacodec support check
        if (state->cur_status == PLAYER_STATUS_PREPARE_START) {
            LOGV("Check hw codec crated or not. vcodec type:%d. hw init:%d \n", state->vdec_type,
                 state->vdec_type == DT_VDEC_TYPE_FFMPEG);
            if (state->vdec_type == DT_VDEC_TYPE_FFMPEG) {
#if 0
                if (dtp->mHWEnable == 1 && dtp->supportMediaCodec()) {
                    dtp->mHWEnable = 0;
                    dtplayer_set_parameter(dtp->mDtpHandle, DTP_CMD_SET_VODEVICE,
                                           dtp->mNativeWindow);
                    dtp->mSeekPosition = dtp->mCurrentPosition;
                    dtplayer_seekto(handle, dtp->mCurrentPosition);
                }
#endif
            }
            goto END;
        }

        memcpy(&dtp->dtp_state, state, sizeof(dtp_state_t));
        if (state->cur_status == PLAYER_STATUS_EXIT) {
            LOGV("PLAYER EXIT OK\n");
            dtp->mListenner->notify(MEDIA_PLAYBACK_COMPLETE);
            dtp->status = PLAYER_EXIT;
            goto END;
        } else if (state->cur_status == PLAYER_STATUS_SEEK_EXIT) {
            dtp->mListenner->notify(MEDIA_SEEK_COMPLETE);
            if (dtp->mCurrentPosition != dtp->mSeekPosition) {
                //still have seek request
                dtp->mSeekPosition = dtp->mCurrentPosition;
                dtplayer_seekto(handle, dtp->mCurrentPosition);
                LOGV("queued seek to %d \n", dtp->mCurrentPosition);
            } else {
                //last seek complete, return to running
                dtp->mSeekPosition = -1;
                LOGV("seek complete !\n");
            }
            goto END;
        }

        if (dtp->status == PLAYER_SEEKING && state->cur_status == PLAYER_STATUS_RUNNING &&
            state->last_status == PLAYER_STATUS_SEEK_EXIT) {
            if (dtp->mSeekPosition > 0) { // receive seek again
                goto END;
            }
            LOGV("set status to running from seek complete \n");
            dtp->status = PLAYER_RUNNING;
            dtp->mListenner->notify(MEDIA_INFO);
        }

        LOGV("UPDATECB CURSTATUS:%x status:%d \n", state->cur_status, dtp->status);
        LOGV("CUR TIME %lld S  FULL TIME:%lld  \n", state->cur_time, state->full_time);
        END:
        unlock(&dtp->dtp_mutex);
        return ret;
    }

} // end namespace android
