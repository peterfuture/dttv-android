/* 
 * porting from ffmpeg to dtplayer 
 *
 * author: peter_future@outlook.com
 *
 * */


#include <android/log.h>

#include <binder/ProcessState.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaBufferGroup.h>
//#include <media/stagefright/MediaDebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/OMXClient.h>
#include <media/stagefright/OMXCodec.h>
#include <utils/List.h>
#include <new>
#include <map>

extern "C" {

#include "vd_wrapper.h"

}

#define TAG "VD-STAGEFRIGHT"

namespace android {
extern "C" {

vd_wrapper_t vd_stagefright_ops;

#define OMX_QCOM_COLOR_FormatYVU420SemiPlanar 0x7FA30C00
#define FF_INPUT_BUFFER_PADDING_SIZE 32
#define TAG "VD-STAGEFRIGHT"

#define vd_stagefright_name "H264 STAGEFRIGHT DECODER"

#define av_freep free
#define av_frame_free free
#define av_mallocz malloc
#define av_malloc malloc

struct Frame {
    status_t status;
    size_t size;
    int64_t time;
    int key;
    uint8_t *buffer;
    dt_av_pic_t *vframe; // for output
};

struct TimeStamp {
    int64_t pts;
    int64_t reordered_opaque;
};

class CustomSource;

typedef struct StagefrightContext {
    uint8_t* orig_extradata;
    int orig_extradata_size;
    sp<MediaSource> *source;
    List<Frame*> *in_queue, *out_queue;
    pthread_mutex_t in_mutex, out_mutex;
    pthread_cond_t condition;
    pthread_t decode_thread_id;

    Frame *end_frame;
    bool source_done;
    volatile sig_atomic_t thread_started, thread_exited, stop_decode;

    dt_av_pic_t *prev_frame;
    std::map<int64_t, TimeStamp> *ts_map;
    int64_t frame_index;

    uint8_t *dummy_buf;
    int dummy_bufsize;

    OMXClient *client;
    sp<MediaSource> *decoder;
    const char *decoder_component;
    int info_changed;
}StagefrightContext;

class CustomSource : public MediaSource {
public:
    CustomSource(dtvideo_decoder_t *decoder, sp<MetaData> meta) {
        dtvideo_para_t *vd_para = &(decoder->para);
        s = (StagefrightContext*)decoder->vd_priv;
        source_meta = meta;
        frame_size  = (vd_para->s_width * vd_para->s_height * 3) / 2;
        buf_group.add_buffer(new MediaBuffer(frame_size));
    }

    virtual sp<MetaData> getFormat() {
        return source_meta;
    }

    virtual status_t start(MetaData *params) {
        return OK;
    }

    virtual status_t stop() {
        return OK;
    }

    virtual status_t read(MediaBuffer **buffer,
                          const MediaSource::ReadOptions *options) {
        Frame *frame;
        status_t ret;

        __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step read, enter read \n");
        if (s->thread_exited)
            return ERROR_END_OF_STREAM;
        pthread_mutex_lock(&s->in_mutex);

        __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step read, enter mutex lock, origsize:%d  \n",s->orig_extradata_size);
        
        while (s->in_queue->empty())
        {
            __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step read, list empty wait \n");
            pthread_cond_wait(&s->condition, &s->in_mutex);
        }
        __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step read, list->size:%d \n",s->in_queue->size());
        frame = *s->in_queue->begin();
        ret = frame->status;

        if (ret == OK) {
            ret = buf_group.acquire_buffer(buffer);
            if (ret == OK) {
                memcpy((*buffer)->data(), frame->buffer, frame->size);
                (*buffer)->set_range(0, frame->size);
                (*buffer)->meta_data()->clear();
                (*buffer)->meta_data()->setInt32(kKeyIsSyncFrame,frame->key);
                (*buffer)->meta_data()->setInt64(kKeyTime, frame->time);
            } else {
                //av_log(s->avctx, AV_LOG_ERROR, "Failed to acquire MediaBuffer\n");
            }
            av_freep(frame->buffer);
        }

        s->in_queue->erase(s->in_queue->begin());
        pthread_mutex_unlock(&s->in_mutex);
        __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step read one frame ok , size:%d key:%d \n",frame->size,frame->key);

        av_freep(frame);
        return ret;
    }

private:
    MediaBufferGroup buf_group;
    sp<MetaData> source_meta;
    StagefrightContext *s;
    int frame_size;
};

static int dt_get_line_size(int pix_fmt, int w, int plane)
{
    if(pix_fmt == DTAV_PIX_FMT_YUV420P)
    {
       switch(plane) 
       {
           case 0:
               return w + 32;
           default:
               return w/2 + 16;
       }
    }

    if(pix_fmt == DTAV_PIX_FMT_NV21)
    {
        switch(plane) 
        {
            case 0:
                return w + 32;
            default:
                return w/2 + 16;
       }
    }

    return -1; // not support
}

void* decode_thread(void *arg)
{
    dtvideo_decoder_t *decoder = (dtvideo_decoder_t *)arg;
    StagefrightContext *s = (StagefrightContext*)decoder->vd_priv;
    Frame* frame;
    MediaBuffer *buffer;
    int32_t w, h;
    int decode_done = 0;
    int ret;
    int src_linesize[3];
    const uint8_t *src_data[3];
    int64_t out_frame_index = 0;
    int pic_size = 0;

    int pix_fmt = decoder->wrapper->para.s_pixfmt;

    do {
        buffer = NULL;
        frame = (Frame*)av_mallocz(sizeof(Frame));
        if (!frame) {
            frame         = s->end_frame;
            frame->status = -1;
            //frame->status = AVERROR(ENOMEM);
            decode_done   = 1;
            s->end_frame  = NULL;
            goto push_frame;
        }
        memset(frame,0,sizeof(Frame));
        __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step start read one frame in decoder\n");
        frame->status = (*s->decoder)->read(&buffer);
        __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step read one frame, status:%d  \n",frame->status);
        if (frame->status == OK) {

            if(buffer->range_length() == 0) // invalid buf, release
            {
                buffer->release();
                buffer = NULL;
                continue;
            }

            sp<MetaData> outFormat = (*s->decoder)->getFormat();
            outFormat->findInt32(kKeyWidth , &w);
            outFormat->findInt32(kKeyHeight, &h);
            frame->vframe = (dt_av_pic_t *)malloc(sizeof(dt_av_pic_t));
            if (!frame->vframe) {
                //frame->status = AVERROR(ENOMEM);
                frame->status = -1;
                decode_done   = 1;
                buffer->release();
                goto push_frame;
            }

            
            if(pix_fmt == DTAV_PIX_FMT_NV21)
            {
                pic_size = w * h + w * h / 2;
            }
            if(pix_fmt == DTAV_PIX_FMT_YUV420P)
            {
                pic_size = w * h + w * h / 2;
            }

            //pic_size = buffer->range_length();

            __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step malloc buffer, size:%di rangesize:%d\n",pic_size,buffer->range_length());
            frame->vframe->data[0] = (uint8_t *)malloc(pic_size);

            // The OMX.SEC decoder doesn't signal the modified width/height
            if (s->decoder_component && !strncmp(s->decoder_component, "OMX.SEC", 7) &&
                (w & 15 || h & 15)) {
                if (((w + 15)&~15) * ((h + 15)&~15) * 3/2 == buffer->range_length()) {
                    w = (w + 15)&~15;
                    h = (h + 15)&~15;
                }
            }

           //line size no need to set
            frame->vframe->linesize[0] = dt_get_line_size(pix_fmt, w, 0);
            frame->vframe->linesize[1] = dt_get_line_size(pix_fmt, w, 1);
            frame->vframe->linesize[2] = dt_get_line_size(pix_fmt, w, 2);

            uint8_t *tmp_buf = (uint8_t*)buffer->data();
            memcpy(frame->vframe->data[0],tmp_buf,pic_size);

            buffer->meta_data()->findInt64(kKeyTime, &out_frame_index);
            if (out_frame_index && s->ts_map->count(out_frame_index) > 0) {
                frame->vframe->pts = (*s->ts_map)[out_frame_index].pts;
                //frame->vframe->reordered_opaque = (*s->ts_map)[out_frame_index].reordered_opaque;
                s->ts_map->erase(out_frame_index);
            }
            buffer->release();
            __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step decoded one frame : pts:%llx outindex:%llx\n",frame->vframe->pts, out_frame_index);
            
        } else if (frame->status == INFO_FORMAT_CHANGED) {
                __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step info chaned :%d \n",frame->status);
                if (buffer)
                    buffer->release();
                av_freep(frame);
                continue;
            } else {
                __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step decode failed, maybe no data left \n");
                usleep(1000);
                continue;
                decode_done = 1;
            }
push_frame:
        while (true) {
            pthread_mutex_lock(&s->out_mutex);
            if (s->out_queue->size() >= 10) {
                pthread_mutex_unlock(&s->out_mutex);
                usleep(10000);
                continue;
            }
            break;
        }
        s->out_queue->push_back(frame);
        pthread_mutex_unlock(&s->out_mutex);
    } while (!decode_done && !s->stop_decode);

    s->thread_exited = true;

    return 0;
}

static int Stagefright_init(dtvideo_decoder_t *decoder)
{
    vd_wrapper_t *wrapper = decoder->wrapper;
    StagefrightContext *s = (StagefrightContext *)malloc(sizeof(StagefrightContext));
    memset(s,0,sizeof(StagefrightContext));
    decoder->vd_priv = s;

    dtvideo_para_t *vd_para = &(decoder->para);

    sp<MetaData> meta, outFormat;
    int32_t colorFormat = 0;
    int pix_fmt;
    int ret;

    if (!vd_para->extradata || !vd_para->extradata_size || vd_para->extradata[0] != 1)
    {
        __android_log_print(ANDROID_LOG_DEBUG,TAG, "NO Valid Extradata Find \n");
        s->orig_extradata_size = 0;
        //return -1;
    }
    else
    {
        s->orig_extradata_size = vd_para->extradata_size;
        s->orig_extradata = (uint8_t*) malloc(vd_para->extradata_size +
                                              FF_INPUT_BUFFER_PADDING_SIZE);
        if (!s->orig_extradata) {
            ret = -1;
            goto fail;
        }
        memcpy(s->orig_extradata, vd_para->extradata, vd_para->extradata_size);
    }

    meta = new MetaData;
    if (meta == NULL) {
        ret = -1;
        goto fail;
    }
    meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_AVC);
    meta->setInt32(kKeyWidth, vd_para->s_width);
    meta->setInt32(kKeyHeight, vd_para->s_height);
    if(s->orig_extradata_size > 0)
        meta->setData(kKeyAVCC, kTypeAVCC, vd_para->extradata, vd_para->extradata_size);

    __android_log_print(ANDROID_LOG_DEBUG,TAG, "meta set ok \n");
    android::ProcessState::self()->startThreadPool();

    s->source    = new sp<MediaSource>();
    *s->source   = new CustomSource(decoder, meta);
    s->in_queue  = new List<Frame*>;
    s->out_queue = new List<Frame*>;
    s->ts_map    = new std::map<int64_t, TimeStamp>;
    s->client    = new OMXClient;
    s->end_frame = (Frame*)malloc(sizeof(Frame));
    if (s->source == NULL || !s->in_queue || !s->out_queue || !s->client ||
        !s->ts_map || !s->end_frame) {
        ret = -1;
        goto fail;
    }

    __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step 1 client->connect \n");
    if (s->client->connect() !=  OK) {
    	__android_log_print(ANDROID_LOG_DEBUG,TAG,"Cannot connect OMX client\n");
        ret = -1;
        goto fail;
    }
    __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step 1 client->connect ok\n");

    s->decoder  = new sp<MediaSource>();
    *s->decoder = OMXCodec::Create(s->client->interface(), meta,
                                  false, *s->source, NULL,
                                  OMXCodec::kClientNeedsFramebuffer);
    if(*s->decoder == NULL)
    {
        __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step 2 create omxcodec failed \n");
        goto fail;
    }
    __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step 2 create omxcodec ok \n");
    if ((*s->decoder)->start() !=  OK) {
    	__android_log_print(ANDROID_LOG_DEBUG,TAG,"Cannot start decoder\n");
        ret = -1;
        s->client->disconnect();
        goto fail;
    }
    __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step 3 start omxcodec ok \n");

    outFormat = (*s->decoder)->getFormat();
    outFormat->findInt32(kKeyColorFormat, &colorFormat);
   
    if (colorFormat == OMX_QCOM_COLOR_FormatYVU420SemiPlanar ||
        colorFormat == OMX_COLOR_FormatYUV420SemiPlanar)
        pix_fmt = DTAV_PIX_FMT_NV21;
    else if (colorFormat == OMX_COLOR_FormatYCbYCr)
        pix_fmt = DTAV_PIX_FMT_YUYV422;
    else if (colorFormat == OMX_COLOR_FormatCbYCrY)
        pix_fmt = DTAV_PIX_FMT_UYVY422;
    else
        pix_fmt = DTAV_PIX_FMT_YUV420P;
    
    __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step 4 get colorFormat info, format:%d pix_fmt:%d  \n", colorFormat, pix_fmt);
    
    outFormat->findCString(kKeyDecoderComponent, &s->decoder_component);
    if (s->decoder_component)
    {
        s->decoder_component = strdup(s->decoder_component);
        __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------component:%s  \n",s->decoder_component);
    }
    pthread_mutex_init(&s->in_mutex, NULL);
    pthread_mutex_init(&s->out_mutex, NULL);
    pthread_cond_init(&s->condition, NULL);
    memcpy(&wrapper->para, &decoder->para, sizeof(dtvideo_para_t));

    if(pix_fmt != wrapper->para.s_pixfmt)
    {
        wrapper->para.s_pixfmt = pix_fmt;
        s->info_changed = 1;
    }

    __android_log_print(ANDROID_LOG_DEBUG,TAG, "vd stagefright init ok \n");
    return 0;

fail:
    //av_bitstream_filter_close(s->bsfc);
    if(s->orig_extradata_size)
        av_freep(&s->orig_extradata);
    av_freep(&s->end_frame);
    delete s->in_queue;
    delete s->out_queue;
    delete s->ts_map;
    delete s->client;
    return ret;
}

static int Stagefright_decode_frame(dtvideo_decoder_t *decoder, dt_av_frame_t *vd_frame,dt_av_pic_t **data)
{
    StagefrightContext *s = (StagefrightContext*)decoder->vd_priv;
    Frame *frame;
    status_t status;
    int orig_size = vd_frame->size;

    dt_av_pic_t *ret_frame;

    if (!s->thread_started) {
        pthread_create(&s->decode_thread_id, NULL, &decode_thread, decoder);
        s->thread_started = true;
        __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step 1 start decode thread ok\n");
    }

    if (!s->source_done) {
        if(!s->dummy_buf) {
            s->dummy_buf = (uint8_t*)av_malloc(vd_frame->size);
            if (!s->dummy_buf)
                return -1;
            s->dummy_bufsize = vd_frame->size;
            memcpy(s->dummy_buf, vd_frame->data, vd_frame->size);
        }

        frame = (Frame*)av_mallocz(sizeof(Frame));
        if (vd_frame->data) {
            frame->status  = OK;
            frame->size    = vd_frame->size;
            frame->key     = vd_frame->key_frame;
            frame->buffer  = (uint8_t*)av_malloc(frame->size);
            if (!frame->buffer) {
                av_freep(&frame);
                return -1;
            }
            uint8_t *ptr = vd_frame->data;
            memcpy(frame->buffer, ptr, orig_size);
            frame->time = ++s->frame_index;
            (*s->ts_map)[s->frame_index].pts = vd_frame->pts;
            //__android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step, fill frame,size:%d  %02x %02x %02x %02x %02x %02x\n",frame->size,frame->buffer[0],frame->buffer[1],frame->buffer[2],frame->buffer[3],frame->buffer[4],frame->buffer[5]);
            //__android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step, fill frame, %02x %02x %02x %02x %02x %02x\n",ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5]);
            //(*s->ts_map)[s->frame_index].reordered_opaque = vd_frame->reordered_opaque;
            __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step, push frame, index:%llx pts:%llx \n",s->frame_index, vd_frame->pts);
        } 
        
        while (true) 
        {
            if (s->thread_exited) {
                __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step decoder thread quit , sourdoen set to 1\n");
                s->source_done = true;
                break;
            }
            pthread_mutex_lock(&s->in_mutex);
            if (s->in_queue->size() >= 10) {
                pthread_mutex_unlock(&s->in_mutex);
                usleep(10000);
                __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step 10 frames in queue, wait decode\n");
                continue;
            }
            s->in_queue->push_back(frame);
            pthread_cond_signal(&s->condition);
            pthread_mutex_unlock(&s->in_mutex);
            __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step push one frame to in queue ok\n");
            break;
        }
    }
#if 0
    while (true) {
        pthread_mutex_lock(&s->out_mutex);
        if (!s->out_queue->empty()) break;
        pthread_mutex_unlock(&s->out_mutex);
        if (s->source_done) {
            usleep(10000);
            continue;
        } else {
            return orig_size;
        }
    }
#endif

    if (s->out_queue->empty())
    {
        __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step have no frame out\n");
        return 0;
    }
    __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step begin to read one frame out\n");

    frame = *s->out_queue->begin();
    s->out_queue->erase(s->out_queue->begin());
    pthread_mutex_unlock(&s->out_mutex);

    ret_frame = frame->vframe;
    status  = frame->status;
    av_freep(&frame);

#if 0
    if (status == ERROR_END_OF_STREAM)
        return 0;
    if (status != OK) {
        //if (status == AVERROR(ENOMEM))
        //    return status;
        //av_log(avctx, AV_LOG_ERROR, "Decode failed: %x\n", status);
        return -1;
    }
#endif
    if (s->prev_frame)
        free(&s->prev_frame);
    s->prev_frame = ret_frame;

    *data = (dt_av_pic_t *)malloc(sizeof(dt_av_pic_t));
    //*got_frame = 1;
    
    memcpy(*data,ret_frame,sizeof(dt_av_pic_t));
    __android_log_print(ANDROID_LOG_DEBUG,TAG, "-------------step decode one frame ok, pts:%lld \n",ret_frame->pts);
    return 1;
    //return orig_size;
}

static int Stagefright_info_changed(dtvideo_decoder_t *decoder)
{
    StagefrightContext *s = (StagefrightContext*)decoder->vd_priv;
    if(s->info_changed) 
    {
        s->info_changed = 0;
        return 1;
    }
    return 0;
}
static int Stagefright_close(dtvideo_decoder_t *decoder)
{
    StagefrightContext *s = (StagefrightContext*)decoder->vd_priv;
    Frame *frame;

    if (s->thread_started) {
        if (!s->thread_exited) {
            s->stop_decode = 1;

            // Make sure decode_thread() doesn't get stuck
            pthread_mutex_lock(&s->out_mutex);
            while (!s->out_queue->empty()) {
                frame = *s->out_queue->begin();
                s->out_queue->erase(s->out_queue->begin());
                if (frame->vframe)
                    av_frame_free(&frame->vframe);
                av_freep(&frame);
            }
            pthread_mutex_unlock(&s->out_mutex);

            // Feed a dummy frame prior to signalling EOF.
            // This is required to terminate the decoder(OMX.SEC)
            // when only one frame is read during stream info detection.
            if (s->dummy_buf && (frame = (Frame*)av_mallocz(sizeof(Frame)))) {
                frame->status = OK;
                frame->size   = s->dummy_bufsize;
                frame->key    = 1;
                frame->buffer = s->dummy_buf;
                pthread_mutex_lock(&s->in_mutex);
                s->in_queue->push_back(frame);
                pthread_cond_signal(&s->condition);
                pthread_mutex_unlock(&s->in_mutex);
                s->dummy_buf = NULL;
            }

            pthread_mutex_lock(&s->in_mutex);
            s->end_frame->status = ERROR_END_OF_STREAM;
            s->in_queue->push_back(s->end_frame);
            pthread_cond_signal(&s->condition);
            pthread_mutex_unlock(&s->in_mutex);
            s->end_frame = NULL;
        }

        pthread_join(s->decode_thread_id, NULL);

        if (s->prev_frame)
            av_frame_free(&s->prev_frame);

        s->thread_started = false;
    }

    while (!s->in_queue->empty()) {
        frame = *s->in_queue->begin();
        s->in_queue->erase(s->in_queue->begin());
        if (frame->size)
            av_freep(&frame->buffer);
        av_freep(&frame);
    }

    while (!s->out_queue->empty()) {
        frame = *s->out_queue->begin();
        s->out_queue->erase(s->out_queue->begin());
        if (frame->vframe)
            av_frame_free(&frame->vframe);
        av_freep(&frame);
    }

    (*s->decoder)->stop();
    s->client->disconnect();

    if (s->decoder_component)
        av_freep(&s->decoder_component);
    av_freep(&s->dummy_buf);
    av_freep(&s->end_frame);

    // Reset the extradata back to the original mp4 format, so that
    // the next invocation (both when decoding and when called from
    // av_find_stream_info) get the original mp4 format extradata.
    //av_freep(&avctx->extradata);
    //avctx->extradata = s->orig_extradata;
    //avctx->extradata_size = s->orig_extradata_size;

    delete s->in_queue;
    delete s->out_queue;
    delete s->ts_map;
    delete s->client;
    delete s->decoder;
    delete s->source;

    pthread_mutex_destroy(&s->in_mutex);
    pthread_mutex_destroy(&s->out_mutex);
    pthread_cond_destroy(&s->condition);
    return 0;
}

void vd_stagefright_init()
{
    vd_stagefright_ops.name = vd_stagefright_name;
    vd_stagefright_ops.vfmt = DT_VIDEO_FORMAT_H264;
    vd_stagefright_ops.type = DT_TYPE_VIDEO;
    vd_stagefright_ops.init = Stagefright_init;
    vd_stagefright_ops.decode_frame = Stagefright_decode_frame;
    vd_stagefright_ops.info_changed = Stagefright_info_changed;
    vd_stagefright_ops.release = Stagefright_close;
    __android_log_print(ANDROID_LOG_DEBUG,TAG, "vd stagefright setup ok \n");
}

}// end extern "C"

}// end namespace
