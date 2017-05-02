#ifndef DTP_AV_H
#define DTP_AV_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/*************************************
** MACRO
*************************************/
#define FILE_NAME_MAX_LENGTH   1024

#define DT_PTS_FREQ            90000
#define DT_PTS_FREQ_MS         90
#define DT_SYNC_DISCONTINUE_THRESHOLD      DT_PTS_FREQ_MS*1000*60
#define DT_SYNC_CORRECT_THRESHOLD          DT_PTS_FREQ_MS*150
//EQUAL TO AV_NOPTS_VALUE IN FFMPEG
#define DT_NOPTS_VALUE         ((int64_t)UINT64_C(0x8000000000000000))

#define PTS_INVALID(x) ((x == -1) || ((int64_t)x == DT_NOPTS_VALUE))
#define PTS_VALID(x) ((x != -1) && (x != DT_NOPTS_VALUE))

#define AVSYNC_THRESHOLD       100     //ms
#define AVSYNC_THRESHOLD_MAX   5*1000  //ms
#define AVSYNC_DROP_THRESHOLD  30*1000 //ms

#define VIDEO_EXTRADATA_SIZE   4096

typedef enum {
    DTP_MEDIA_TYPE_UNKNOWN = -1,
    DTP_MEDIA_TYPE_VIDEO,
    DTP_MEDIA_TYPE_AUDIO,
    DTP_MEDIA_TYPE_DATA,
    DTP_MEDIA_TYPE_SUBTITLE,
    DTP_MEDIA_TYPE_ATTACHMENT,
    DTP_MEDIA_TYPE_NB
}
dtp_media_type_t;

typedef enum {
    DTP_MEDIA_FORMAT_INVALID = -1,
    DTP_MEDIA_FORMAT_MPEGTS,
    DTP_MEDIA_FORMAT_MPEGPS,
    DTP_MEDIA_FORMAT_RM,
    DTP_MEDIA_FORMAT_AVI,
    DTP_MEDIA_FORMAT_MKV,
    DTP_MEDIA_FORMAT_MOV,
    DTP_MEDIA_FORMAT_MP4,
    DTP_MEDIA_FORMAT_FLV,
    DTP_MEDIA_FORMAT_AAC,
    DTP_MEDIA_FORMAT_AC3,
    DTP_MEDIA_FORMAT_MP3,
    DTP_MEDIA_FORMAT_WAV,
    DTP_MEDIA_FORMAT_DTS,
    DTP_MEDIA_FORMAT_FLAC,
    DTP_MEDIA_FORMAT_H264,
    DTP_MEDIA_FORMAT_AVS,
    DTP_MEDIA_FORMAT_M2V,
    DTP_MEDIA_FORMAT_P2P,
    DTP_MEDIA_FORMAT_ASF,
    DTP_MEDIA_FORMAT_RTSP,
    DTP_MEDIA_FORMAT_APE,
    DTP_MEDIA_FORMAT_AMR,
    DTP_MEDIA_FORMAT_UNKOWN,
} dtp_media_format_t;

/* av packet to be decoded */
typedef struct dt_av_pkt {
    uint8_t *data;
    int size;
    int64_t pts;
    int64_t dts;
    int duration;
    int key_frame;
    dtp_media_type_t type;
} dt_av_pkt_t;


enum {
    DTP_FRAME_FLAG_NONE       = 0,
    DTP_FRAME_FLAG_FFMPEG     = 1,
    DTP_FRAME_FLAG_MEDIACODEC = 2,
};

/* av frame after decode */
typedef struct {
    /* frame members need to equal to ffmpeg AVFrame*/
    uint8_t *data[8];
    int linesize[8];

    /* property */
    int width;
    int height;
    int pixfmt;

    int64_t pts;
    int64_t dts;
    int duration;
    int flags;
    void *opaque;
} dt_av_frame_t;


// =================================================
// VIDEO
typedef enum {
    DT_VIDEO_FORMAT_INVALID = -1,
    DT_VIDEO_FORMAT_MPEG2,
    DT_VIDEO_FORMAT_MPEG4,
    DT_VIDEO_FORMAT_H264,
    DT_VIDEO_FORMAT_HEVC,
    DT_VIDEO_FORMAT_UNKOWN,
} dtvideo_format_t;

typedef enum {
    DTAV_FLAG_NONE = 0x0,
    DTAV_FLAG_DISABLE_HW_CODEC = 0x1,
} dtvideo_flag_t;

enum video_render_mode {
    VIDEO_RENDER_MODE_PTS = 0,
    VIDEO_RENDER_MODE_DURATION,
};

//From ffmpeg
typedef enum DT_AVPixelFormat {
    DTAV_PIX_FMT_NONE = -1,
    DTAV_PIX_FMT_YUV420P,       ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    DTAV_PIX_FMT_YUYV422,       ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    DTAV_PIX_FMT_RGB24,         ///< packed RGB 8:8:8, 24bpp, RGBRGB...
    DTAV_PIX_FMT_BGR24,         ///< packed RGB 8:8:8, 24bpp, BGRBGR...
    DTAV_PIX_FMT_YUV422P,       ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    DTAV_PIX_FMT_YUV444P,       ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    DTAV_PIX_FMT_YUV410P,       ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    DTAV_PIX_FMT_YUV411P,       ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    DTAV_PIX_FMT_GRAY8,         ///<        Y        ,  8bpp
    DTAV_PIX_FMT_MONOWHITE,     ///<        Y        ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
    DTAV_PIX_FMT_MONOBLACK,     ///<        Y        ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
    DTAV_PIX_FMT_PAL8,          ///< 8 bit with PIX_FMT_RGB32 palette
    DTAV_PIX_FMT_YUVJ420P,      ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV420P and setting color_range
    DTAV_PIX_FMT_YUVJ422P,      ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV422P and setting color_range
    DTAV_PIX_FMT_YUVJ444P,      ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV444P and setting color_range
    DTAV_PIX_FMT_XVMC_MPEG2_MC, ///< XVideo Motion Acceleration via common packet passing
    DTAV_PIX_FMT_XVMC_MPEG2_IDCT,
    DTAV_PIX_FMT_UYVY422,       ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    DTAV_PIX_FMT_UYYVYY411,     ///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    DTAV_PIX_FMT_BGR8,          ///< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    DTAV_PIX_FMT_BGR4,          ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    DTAV_PIX_FMT_BGR4_BYTE,     ///< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    DTAV_PIX_FMT_RGB8,          ///< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    DTAV_PIX_FMT_RGB4,          ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    DTAV_PIX_FMT_RGB4_BYTE,     ///< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
    DTAV_PIX_FMT_NV12,          ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    DTAV_PIX_FMT_NV21,          ///< as above, but U and V bytes are swapped

    DTAV_PIX_FMT_ARGB,          ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    DTAV_PIX_FMT_RGBA,          ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    DTAV_PIX_FMT_ABGR,          ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    DTAV_PIX_FMT_BGRA,          ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...

    DTAV_PIX_FMT_GRAY16BE,      ///<        Y        , 16bpp, big-endian
    DTAV_PIX_FMT_GRAY16LE,      ///<        Y        , 16bpp, little-endian
    DTAV_PIX_FMT_YUV440P,       ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    DTAV_PIX_FMT_YUVJ440P,      ///< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of PIX_FMT_YUV440P and setting color_range
    DTAV_PIX_FMT_YUVA420P,      ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
    DTAV_PIX_FMT_VDPAU_H264,    ///< H.264 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    DTAV_PIX_FMT_VDPAU_MPEG1,   ///< MPEG-1 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    DTAV_PIX_FMT_VDPAU_MPEG2,   ///< MPEG-2 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    DTAV_PIX_FMT_VDPAU_WMV3,    ///< WMV3 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    DTAV_PIX_FMT_VDPAU_VC1,     ///< VC-1 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    DTAV_PIX_FMT_RGB48BE,       ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
    DTAV_PIX_FMT_RGB48LE,       ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian

    DTAV_PIX_FMT_RGB565BE,      ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian
    DTAV_PIX_FMT_RGB565LE,      ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian
    DTAV_PIX_FMT_RGB555BE,      ///< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), big-endian, most significant bit to 0
    DTAV_PIX_FMT_RGB555LE,      ///< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), little-endian, most significant bit to 0

    DTAV_PIX_FMT_BGR565BE,      ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian
    DTAV_PIX_FMT_BGR565LE,      ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian
    DTAV_PIX_FMT_BGR555BE,      ///< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), big-endian, most significant bit to 1
    DTAV_PIX_FMT_BGR555LE,      ///< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), little-endian, most significant bit to 1

    DTAV_PIX_FMT_VAAPI_MOCO,    ///< HW acceleration through VA API at motion compensation entry-point, Picture.data[3] contains a vaapi_render_state struct which contains macroblocks as well as various fields extracted from headers
    DTAV_PIX_FMT_VAAPI_IDCT,    ///< HW acceleration through VA API at IDCT entry-point, Picture.data[3] contains a vaapi_render_state struct which contains fields extracted from headers
    DTAV_PIX_FMT_VAAPI_VLD,     ///< HW decoding through VA API, Picture.data[3] contains a vaapi_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers

    DTAV_PIX_FMT_YUV420P16LE,   ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    DTAV_PIX_FMT_YUV420P16BE,   ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    DTAV_PIX_FMT_YUV422P16LE,   ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    DTAV_PIX_FMT_YUV422P16BE,   ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    DTAV_PIX_FMT_YUV444P16LE,   ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    DTAV_PIX_FMT_YUV444P16BE,   ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    DTAV_PIX_FMT_VDPAU_MPEG4,   ///< MPEG4 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    DTAV_PIX_FMT_DXVA2_VLD,     ///< HW decoding through DXVA2, Picture.data[3] contains a LPDIRECT3DSURFACE9 pointer

    DTAV_PIX_FMT_RGB444LE,      ///< packed RGB 4:4:4, 16bpp, (msb)4A 4R 4G 4B(lsb), little-endian, most significant bits to 0
    DTAV_PIX_FMT_RGB444BE,      ///< packed RGB 4:4:4, 16bpp, (msb)4A 4R 4G 4B(lsb), big-endian, most significant bits to 0
    DTAV_PIX_FMT_BGR444LE,      ///< packed BGR 4:4:4, 16bpp, (msb)4A 4B 4G 4R(lsb), little-endian, most significant bits to 1
    DTAV_PIX_FMT_BGR444BE,      ///< packed BGR 4:4:4, 16bpp, (msb)4A 4B 4G 4R(lsb), big-endian, most significant bits to 1
    DTAV_PIX_FMT_GRAY8A,        ///< 8bit gray, 8bit alpha
    DTAV_PIX_FMT_BGR48BE,       ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
    DTAV_PIX_FMT_BGR48LE,       ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian

    //the following 10 formats have the disadvantage of needing 1 format for each bit depth, thus
    //If you want to support multiple bit depths, then using DTAV_PIX_FMT_YUV420P16* with the bpp stored separately
    //is better
    DTAV_PIX_FMT_YUV420P9BE,    ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    DTAV_PIX_FMT_YUV420P9LE,    ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    DTAV_PIX_FMT_YUV420P10BE,   ///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    DTAV_PIX_FMT_YUV420P10LE,   ///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    DTAV_PIX_FMT_YUV422P10BE,   ///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    DTAV_PIX_FMT_YUV422P10LE,   ///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    DTAV_PIX_FMT_YUV444P9BE,    ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    DTAV_PIX_FMT_YUV444P9LE,    ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    DTAV_PIX_FMT_YUV444P10BE,   ///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    DTAV_PIX_FMT_YUV444P10LE,   ///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    DTAV_PIX_FMT_YUV422P9BE,    ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    DTAV_PIX_FMT_YUV422P9LE,    ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    DTAV_PIX_FMT_VDA_VLD,       ///< hardware decoding through VDA

#ifdef DTAV_PIX_FMT_ABI_GIT_MASTER
    DTAV_PIX_FMT_RGBA64BE,      ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    DTAV_PIX_FMT_RGBA64LE,      ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    DTAV_PIX_FMT_BGRA64BE,      ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    DTAV_PIX_FMT_BGRA64LE,      ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
#endif
    DTAV_PIX_FMT_GBRP,          ///< planar GBR 4:4:4 24bpp
    DTAV_PIX_FMT_GBRP9BE,       ///< planar GBR 4:4:4 27bpp, big-endian
    DTAV_PIX_FMT_GBRP9LE,       ///< planar GBR 4:4:4 27bpp, little-endian
    DTAV_PIX_FMT_GBRP10BE,      ///< planar GBR 4:4:4 30bpp, big-endian
    DTAV_PIX_FMT_GBRP10LE,      ///< planar GBR 4:4:4 30bpp, little-endian
    DTAV_PIX_FMT_GBRP16BE,      ///< planar GBR 4:4:4 48bpp, big-endian
    DTAV_PIX_FMT_GBRP16LE,      ///< planar GBR 4:4:4 48bpp, little-endian

    /**
        * duplicated pixel formats for compatibility with libav.
        * FFmpeg supports these formats since May 8 2012 and Jan 28 2012 (commits f9ca1ac7 and 143a5c55)
        * Libav added them Oct 12 2012 with incompatible values (commit 6d5600e85)
        */
    DTAV_PIX_FMT_YUVA422P_LIBAV, ///< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
    DTAV_PIX_FMT_YUVA444P_LIBAV, ///< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)

    DTAV_PIX_FMT_YUVA420P9BE,   ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian
    DTAV_PIX_FMT_YUVA420P9LE,   ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian
    DTAV_PIX_FMT_YUVA422P9BE,   ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian
    DTAV_PIX_FMT_YUVA422P9LE,   ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian
    DTAV_PIX_FMT_YUVA444P9BE,   ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    DTAV_PIX_FMT_YUVA444P9LE,   ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    DTAV_PIX_FMT_YUVA420P10BE,  ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    DTAV_PIX_FMT_YUVA420P10LE,  ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    DTAV_PIX_FMT_YUVA422P10BE,  ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    DTAV_PIX_FMT_YUVA422P10LE,  ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    DTAV_PIX_FMT_YUVA444P10BE,  ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    DTAV_PIX_FMT_YUVA444P10LE,  ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    DTAV_PIX_FMT_YUVA420P16BE,  ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    DTAV_PIX_FMT_YUVA420P16LE,  ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    DTAV_PIX_FMT_YUVA422P16BE,  ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    DTAV_PIX_FMT_YUVA422P16LE,  ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    DTAV_PIX_FMT_YUVA444P16BE,  ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    DTAV_PIX_FMT_YUVA444P16LE,  ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)

    DTAV_PIX_FMT_VDPAU,         ///< HW acceleration through VDPAU, Picture.data[3] contains a VdpVideoSurface

#ifndef DTAV_PIX_FMT_ABI_GIT_MASTER
    DTAV_PIX_FMT_RGBA64BE = 0x123, ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    DTAV_PIX_FMT_RGBA64LE,      ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    DTAV_PIX_FMT_BGRA64BE,      ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    DTAV_PIX_FMT_BGRA64LE,      ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
#endif
    DTAV_PIX_FMT_0RGB = 0x123 + 4, ///< packed RGB 8:8:8, 32bpp, 0RGB0RGB...
    DTAV_PIX_FMT_RGB0,          ///< packed RGB 8:8:8, 32bpp, RGB0RGB0...
    DTAV_PIX_FMT_0BGR,          ///< packed BGR 8:8:8, 32bpp, 0BGR0BGR...
    DTAV_PIX_FMT_BGR0,          ///< packed BGR 8:8:8, 32bpp, BGR0BGR0...
    DTAV_PIX_FMT_YUVA444P,      ///< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
    DTAV_PIX_FMT_YUVA422P,      ///< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)

    DTAV_PIX_FMT_YUV420P12BE,   ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    DTAV_PIX_FMT_YUV420P12LE,   ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    DTAV_PIX_FMT_YUV420P14BE,   ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    DTAV_PIX_FMT_YUV420P14LE,   ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    DTAV_PIX_FMT_YUV422P12BE,   ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    DTAV_PIX_FMT_YUV422P12LE,   ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    DTAV_PIX_FMT_YUV422P14BE,   ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    DTAV_PIX_FMT_YUV422P14LE,   ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    DTAV_PIX_FMT_YUV444P12BE,   ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    DTAV_PIX_FMT_YUV444P12LE,   ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    DTAV_PIX_FMT_YUV444P14BE,   ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    DTAV_PIX_FMT_YUV444P14LE,   ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    DTAV_PIX_FMT_GBRP12BE,      ///< planar GBR 4:4:4 36bpp, big-endian
    DTAV_PIX_FMT_GBRP12LE,      ///< planar GBR 4:4:4 36bpp, little-endian
    DTAV_PIX_FMT_GBRP14BE,      ///< planar GBR 4:4:4 42bpp, big-endian
    DTAV_PIX_FMT_GBRP14LE,      ///< planar GBR 4:4:4 42bpp, little-endian
    DTAV_PIX_FMT_NB,            ///< number of pixel formats, DO NOT USE THIS if you want to link with shared libav* because the number of formats might differ between versions

} dtp_pixfmt_t;

enum {
    DT_SCREEN_MODE_NORMAL,
    DT_SCREEN_MODE_FULL,
    DT_SCREEN_MODE_4_3,
    DT_SCREEN_MODE_16_9
};

typedef enum {
    VO_ID_NULL = 0,
    VO_ID_EX,            // ex vo need set to 0 default
    VO_ID_SDL,
    VO_ID_X11,
    VO_ID_FB,
    VO_ID_GL,
    VO_ID_DIRECTX,
    VO_ID_SDL2,
    VO_ID_ANDROID_OPENGL = 0x100,
    VO_ID_ANDROID_SURFACE = 0x101,
    VO_ID_IOS = 0x200,
} dtp_vo_t;

// =================================================
// AUDIO
typedef enum {
    DT_AUDIO_FORMAT_INVALID = -1,
    DT_AUDIO_FORMAT_MP2,
    DT_AUDIO_FORMAT_MP3,
    DT_AUDIO_FORMAT_AAC,
    DT_AUDIO_FORMAT_AC3,
    DT_AUDIO_FORMAT_UNKOWN,
} dtaudio_format_t;

typedef enum _AO_ID_ {
    AO_ID_EXAMPLE = -1,
    AO_ID_EX,                   // ex ao set to 0 default
    AO_ID_ALSA,                 // 0 default
    AO_ID_SDL,
    AO_ID_OSS,
    AO_ID_DSOUND,
    AO_ID_ANDROID = 0x100,
    AO_ID_OPENSL = 0x101,
    AO_ID_IOS = 0x200,
} dtao_format_t;

// =================================================
// SUBTITLE
typedef enum {
    DT_SUB_FORMAT_INVALID = -1,
    DT_SUB_FORMAT_DVD_SUB,
    DT_SUB_FORMAT_DVB_SUB,
    DT_SUB_FORMAT_UNKOWN,
} dtsub_format_t;

typedef enum dtav_sub_type {
    DT_SUBTITLE_NONE,

    DT_SUBTITLE_BITMAP,                ///< A bitmap, pict will be set

    /**
     * Plain text, the text field must be set by the decoder and is
     * authoritative. ass and pict fields may contain approximations.
     */
    DT_SUBTITLE_TEXT,

    /**
     * Formatted text, the ass field must be set by the decoder and is
     * authoritative. pict and text fields may contain approximations.
     */
    DT_SUBTITLE_ASS,
} dtav_sub_type_t;


typedef struct dt_sub_Rect {
    int x;         ///< top left corner  of pict, undefined when pict is not set
    int y;         ///< top left corner  of pict, undefined when pict is not set
    int w;         ///< width            of pict, undefined when pict is not set
    int h;         ///< height           of pict, undefined when pict is not set
    int nb_colors; ///< number of colors in pict, undefined when pict is not set

    /**
     * data+linesize for the bitmap of this subtitle.
     * can be set for text/ass as well once they where rendered
     */
    //AVPicture pict;
    dt_av_frame_t pict;
    dtav_sub_type_t type;

    char *text;                     ///< 0 terminated plain UTF-8 text

    /**
     * 0 terminated ASS/SSA compatible event line.
     * The presentation of this is unaffected by the other values in this
     * struct.
     */
    char *ass;

    int flags;
} dtav_sub_rect_t;

typedef struct dtav_subtitle {
    uint16_t format; /* 0 = graphics */
    uint32_t start_display_time; /* relative to packet pts, in ms */
    uint32_t end_display_time; /* relative to packet pts, in ms */
    unsigned num_rects;
    dtav_sub_rect_t **rects;
    int64_t pts;    ///< Same as packet pts, in AV_TIME_BASE
} dtav_sub_frame_t;

dt_av_frame_t *dtp_frame_alloc();
void dtp_frame_unref(dt_av_frame_t *frame, int render);
void dtp_frame_free(dt_av_frame_t *frame, int render);

#ifdef  __cplusplus
}
#endif

#endif
