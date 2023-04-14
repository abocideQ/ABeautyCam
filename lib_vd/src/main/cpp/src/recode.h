//
// Created by 86193 on 2023/3/24.
//

#ifndef AVEDITOR_RECODE_H
#define AVEDITOR_RECODE_H

#include "common.h"

typedef struct av_config {
    AVCodecID AVVideoCodecID = AV_CODEC_ID_NONE;
    int v_width = 0;
    int v_height = 0;
    int v_gop_size = 0;
    int v_bit_rate = 0;
    int v_fps = 0;
    AVCodecID AVAudioCodecID = AV_CODEC_ID_NONE;
    int a_sample_rate = 0;
    int a_bit_rate = 0;
    int a_ch_layout = 0;
    long long time_start = 0;
    long long time_end = 0;
} AVConfig;

typedef struct av_stream_model {
    int stream_index;
    AVMediaType codec_type;
    AVCodecContext *in_av_decode_ctx;
    SwsContext *in_sws_ctx;
    SwrContext *in_swr_ctx;
    AVCodecContext *out_av_decode_ctx;
    int64_t next_pts;
} AVStreamModel;

class recode {
public:
    int recode_codec(const std::string &,
                     const std::string &,
                     const AVConfig &);

private:
    AVFormatContext *m_in_avformat_ctx;
    AVPacket *m_packet;
    AVFrame *m_frame;
    AVFormatContext *m_out_avformat_ctx;
    int m_av_stream_models_size;
    AVStreamModel **m_av_stream_models;

    void recode_free();
};

#endif //AVEDITOR_RECODE_H
