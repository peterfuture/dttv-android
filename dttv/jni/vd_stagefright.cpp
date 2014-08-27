/* 
 * porting from ffmpeg to dtplayer 
 *
 * author: peter_future@outlook.com
 *
 * */


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
    AVPicture_t *vframe; // for output
    //AVFrame *vframe;
};

struct TimeStamp {
    int64_t pts;
    int64_t reordered_opaque;
};

class CustomSource;

struct StagefrightContext {
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

    AVPicture_t *prev_frame;
    std::map<int64_t, TimeStamp> *ts_map;
    int64_t frame_index;

    uint8_t *dummy_buf;
    int dummy_bufsize;

    OMXClient *client;
    sp<MediaSource> *decoder;
    const char *decoder_component;
};

class CustomSource : public MediaSource {
public:
    CustomSource(vd_wrapper_t *wrapper, sp<MetaData> meta) {
        dtvideo_decoder_t *decoder = (dtvideo_decoder_t *)wrapper->parent;
        dtvideo_para_t *vd_para = &(decoder->para);
        s = (StagefrightContext*)wrapper->vd_priv;
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

        if (s->thread_exited)
            return ERROR_END_OF_STREAM;
        pthread_mutex_lock(&s->in_mutex);

        while (s->in_queue->empty())
            pthread_cond_wait(&s->condition, &s->in_mutex);

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
            av_freep(&frame->buffer);
        }

        s->in_queue->erase(s->in_queue->begin());
        pthread_mutex_unlock(&s->in_mutex);

        av_freep(&frame);
        return ret;
    }

private:
    MediaBufferGroup buf_group;
    sp<MetaData> source_meta;
    StagefrightContext *s;
    int frame_size;
};

void* decode_thread(void *arg)
{
    vd_wrapper_t *wrapper = (vd_wrapper_t*)arg;
    StagefrightContext *s = (StagefrightContext*)wrapper->vd_priv;
    //const AVPixFmtDescriptor *pix_desc = av_pix_fmt_desc_get(avctx->pix_fmt);
    //const AVPixFmtDescriptor *pix_desc = DTAV_PIX_FMT_YUV420P;
    Frame* frame;
    MediaBuffer *buffer;
    int32_t w, h;
    int decode_done = 0;
    int ret;
    int src_linesize[3];
    const uint8_t *src_data[3];
    int64_t out_frame_index = 0;

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
        frame->status = (*s->decoder)->read(&buffer);
        if (frame->status == OK) {
            sp<MetaData> outFormat = (*s->decoder)->getFormat();
            outFormat->findInt32(kKeyWidth , &w);
            outFormat->findInt32(kKeyHeight, &h);
            //frame->vframe = av_frame_alloc();
            frame->vframe = (AVPicture_t *)malloc(sizeof(AVPicture_t));
            if (!frame->vframe) {
                //frame->status = AVERROR(ENOMEM);
                frame->status = -1;
                decode_done   = 1;
                buffer->release();
                goto push_frame;
            }
#if 0
            ret = ff_get_buffer(avctx, frame->vframe, AV_GET_BUFFER_FLAG_REF);
            if (ret < 0) {
                frame->status = ret;
                decode_done   = 1;
                buffer->release();
                goto push_frame;
            }
#endif
            frame->vframe->data[0] = (uint8_t *)malloc(sizeof(w*h*2));

            // The OMX.SEC decoder doesn't signal the modified width/height
            if (s->decoder_component && !strncmp(s->decoder_component, "OMX.SEC", 7) &&
                (w & 15 || h & 15)) {
                if (((w + 15)&~15) * ((h + 15)&~15) * 3/2 == buffer->range_length()) {
                    w = (w + 15)&~15;
                    h = (h + 15)&~15;
                }
            }
#if 0
            if (!avctx->width || !avctx->height || avctx->width > w || avctx->height > h) {
                avctx->width  = w;
                avctx->height = h;
            }
#endif
#if 0
            src_linesize[0] = av_image_get_linesize(avctx->pix_fmt, w, 0);
            src_linesize[1] = av_image_get_linesize(avctx->pix_fmt, w, 1);
            src_linesize[2] = av_image_get_linesize(avctx->pix_fmt, w, 2);
#endif
            src_linesize[0] = w*h;
            src_linesize[1] = w*h/4;
            src_linesize[2] = w*h/4;

            src_data[0] = (uint8_t*)buffer->data();
            src_data[1] = src_data[0] + src_linesize[0] * h;
            //src_data[2] = src_data[1] + src_linesize[1] * -(-h>>pix_desc->log2_chroma_h);
#if 0
            av_image_copy(frame->vframe->data, frame->vframe->linesize,
                          src_data, src_linesize,
                          avctx->pix_fmt, avctx->width, avctx->height);
#endif
            buffer->meta_data()->findInt64(kKeyTime, &out_frame_index);
            if (out_frame_index && s->ts_map->count(out_frame_index) > 0) {
                frame->vframe->pts = (*s->ts_map)[out_frame_index].pts;
                //frame->vframe->reordered_opaque = (*s->ts_map)[out_frame_index].reordered_opaque;
                s->ts_map->erase(out_frame_index);
            }
            buffer->release();
            } else if (frame->status == INFO_FORMAT_CHANGED) {
                if (buffer)
                    buffer->release();
                av_freep(frame);
                continue;
            } else {
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


static int Stagefright_init(vd_wrapper_t *wrapper, void *parent)
{
    dtvideo_decoder_t *decoder = (dtvideo_decoder_t *)parent;
    wrapper->parent = decoder;

    StagefrightContext *s = (StagefrightContext *)malloc(sizeof(StagefrightContext));
    wrapper->vd_priv = s;

    dtvideo_para_t *vd_para = &(decoder->para);

    sp<MetaData> meta, outFormat;
    int32_t colorFormat = 0;
    int ret;

    if (!vd_para->extradata || !vd_para->extradata_size || vd_para->extradata[0] != 1)
        return -1;

#if 0 // here maybe have err
    s->avctx = avctx;
    s->bsfc  = av_bitstream_filter_init("h264_mp4toannexb");
    if (!s->bsfc) {
    	__android_log_print(ANDROID_LOG_DEBUG,TAG,"Cannot open the h264_mp4toannexb BSF!\n");
        return -1;
    }
#endif
    s->orig_extradata_size = vd_para->extradata_size;
    s->orig_extradata = (uint8_t*) malloc(vd_para->extradata_size +
                                              FF_INPUT_BUFFER_PADDING_SIZE);
    if (!s->orig_extradata) {
        ret = -1;
        goto fail;
    }
    memcpy(s->orig_extradata, vd_para->extradata, vd_para->extradata_size);

    meta = new MetaData;
    if (meta == NULL) {
        ret = -1;
        goto fail;
    }
    meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_AVC);
    meta->setInt32(kKeyWidth, vd_para->s_width);
    meta->setInt32(kKeyHeight, vd_para->s_height);
    meta->setData(kKeyAVCC, kTypeAVCC, vd_para->extradata, vd_para->extradata_size);

    android::ProcessState::self()->startThreadPool();

    s->source    = new sp<MediaSource>();
    *s->source   = new CustomSource(wrapper, meta);
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

    if (s->client->connect() !=  OK) {
    	__android_log_print(ANDROID_LOG_DEBUG,TAG,"Cannot connect OMX client\n");
        ret = -1;
        goto fail;
    }

    s->decoder  = new sp<MediaSource>();
    *s->decoder = OMXCodec::Create(s->client->interface(), meta,
                                  false, *s->source, NULL,
                                  OMXCodec::kClientNeedsFramebuffer);
    if ((*s->decoder)->start() !=  OK) {
    	__android_log_print(ANDROID_LOG_DEBUG,TAG,"Cannot start decoder\n");
        ret = -1;
        s->client->disconnect();
        goto fail;
    }

    outFormat = (*s->decoder)->getFormat();
    outFormat->findInt32(kKeyColorFormat, &colorFormat);
#if 0
    if (colorFormat == OMX_QCOM_COLOR_FormatYVU420SemiPlanar ||
        colorFormat == OMX_COLOR_FormatYUV420SemiPlanar)
        avctx->pix_fmt = DTAV_PIX_FMT_NV21;
    else if (colorFormat == OMX_COLOR_FormatYCbYCr)
        avctx->pix_fmt = DTAV_PIX_FMT_YUYV422;
    else if (colorFormat == OMX_COLOR_FormatCbYCrY)
        avctx->pix_fmt = DTAV_PIX_FMT_UYVY422;
    else
        avctx->pix_fmt = DTAV_PIX_FMT_YUV420P;
#endif
    outFormat->findCString(kKeyDecoderComponent, &s->decoder_component);
    if (s->decoder_component)
        s->decoder_component = strdup(s->decoder_component);

    pthread_mutex_init(&s->in_mutex, NULL);
    pthread_mutex_init(&s->out_mutex, NULL);
    pthread_cond_init(&s->condition, NULL);
    return 0;

fail:
    //av_bitstream_filter_close(s->bsfc);
    av_freep(&s->orig_extradata);
    av_freep(&s->end_frame);
    delete s->in_queue;
    delete s->out_queue;
    delete s->ts_map;
    delete s->client;
    return ret;
}

static int Stagefright_decode_frame(vd_wrapper_t *wrapper, dt_av_frame_t *vd_frame,AVPicture_t **data)
{
    dtvideo_decoder_t *decoder = (dtvideo_decoder_t *)wrapper->parent;
    StagefrightContext *s = (StagefrightContext*)wrapper->vd_priv;
    Frame *frame;
    status_t status;
    int orig_size = vd_frame->size;

#if 0
    AVPacket pkt = *avpkt;
    AVFrame *ret_frame;
#endif
    AVPicture_t *ret_frame;
    
    if (!s->thread_started) {
        pthread_create(&s->decode_thread_id, NULL, &decode_thread, wrapper);
        s->thread_started = true;
    }
#if 0
    if (avpkt && avpkt->data) {
        av_bitstream_filter_filter(s->bsfc, avctx, NULL, &pkt.data, &pkt.size,
                                   avpkt->data, avpkt->size, avpkt->flags & AV_PKT_FLAG_KEY);
        avpkt = &pkt;
    }
#endif
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
            frame->buffer  = (uint8_t*)av_malloc(vd_frame->size);
            if (!frame->buffer) {
                av_freep(&frame);
                return -1;
            }
            uint8_t *ptr = vd_frame->data;
            
#if 0
            // The OMX.SEC decoder fails without this.
            if (avpkt->size == orig_size + vd_frame->extradata_size) {
                ptr += avctx->extradata_size;
                frame->size = orig_size;
            }
#endif
            memcpy(frame->buffer, ptr, orig_size);
#if 0
            if (avpkt == &pkt)
                av_free(avpkt->data);
#endif
            frame->time = ++s->frame_index;
            (*s->ts_map)[s->frame_index].pts = vd_frame->pts;
            //(*s->ts_map)[s->frame_index].reordered_opaque = vd_frame->reordered_opaque;
        } else {
            frame->status  = ERROR_END_OF_STREAM;
            s->source_done = true;
        }

        while (true) {
            if (s->thread_exited) {
                s->source_done = true;
                break;
            }
            pthread_mutex_lock(&s->in_mutex);
            if (s->in_queue->size() >= 10) {
                pthread_mutex_unlock(&s->in_mutex);
                usleep(10000);
                continue;
            }
            s->in_queue->push_back(frame);
            pthread_cond_signal(&s->condition);
            pthread_mutex_unlock(&s->in_mutex);
            break;
        }
    }
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

    frame = *s->out_queue->begin();
    s->out_queue->erase(s->out_queue->begin());
    pthread_mutex_unlock(&s->out_mutex);

    ret_frame = frame->vframe;
    status  = frame->status;
    av_freep(&frame);

    if (status == ERROR_END_OF_STREAM)
        return 0;
    if (status != OK) {
        //if (status == AVERROR(ENOMEM))
        //    return status;
        //av_log(avctx, AV_LOG_ERROR, "Decode failed: %x\n", status);
        return -1;
    }

    if (s->prev_frame)
        free(&s->prev_frame);
    s->prev_frame = ret_frame;

    //*got_frame = 1;
    *(AVPicture_t*)data = *ret_frame;
    return orig_size;
}

static int Stagefright_close(vd_wrapper_t *wrapper)
{
    dtvideo_decoder_t *decoder = (dtvideo_decoder_t *)wrapper->parent;
    StagefrightContext *s = (StagefrightContext*)wrapper->vd_priv;
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

void android_vd_init()
{
    vd_stagefright_ops.name = vd_stagefright_name;
    vd_stagefright_ops.vfmt = DT_VIDEO_FORMAT_H264;
    vd_stagefright_ops.type = DT_TYPE_VIDEO;
    vd_stagefright_ops.init = Stagefright_init;
    vd_stagefright_ops.decode_frame = Stagefright_decode_frame;
    vd_stagefright_ops.release = Stagefright_close;
}

}// end extern "C"

}// end namespace
