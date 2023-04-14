//
// Created by 86193 on 2023/3/21.
//
#include "repack.h"

int repack::repack_fmt(const std::string &in_url,
                       const std::string &out_url) {
    m_inAVFormatContext = avformat_alloc_context();
    if (avformat_open_input(&m_inAVFormatContext,
                            in_url.c_str(),
                            nullptr,
                            nullptr) < 0) {
        goto __ERR;
    }
    if (avformat_find_stream_info(m_inAVFormatContext, nullptr) < 0) {
        goto __ERR;
    }
    m_outAVFormatContext = avformat_alloc_context();
    if (avformat_alloc_output_context2(&m_outAVFormatContext,
                                       nullptr,
                                       nullptr,
                                       out_url.c_str()) < 0) {
        goto __ERR;
    }
    {
        //stream_index
        m_stream_index = 0;
        m_stream_mapping_size = (int) m_inAVFormatContext->nb_streams;
        m_stream_mapping = (int *) av_mallocz_array(m_stream_mapping_size,
                                                    sizeof(*m_stream_mapping));
    }
    for (size_t i = 0; i < m_inAVFormatContext->nb_streams; i++) {
        AVStream *in_stream = m_inAVFormatContext->streams[i];
        AVStream *out_stream = avformat_new_stream(m_outAVFormatContext, nullptr);
        if (out_stream == nullptr) {
            goto __ERR;
        }
        {
            //stream_index
            if (in_stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
                in_stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
                in_stream->codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
                m_stream_mapping[i] = -1;
                continue;
            }
            m_stream_mapping[i] = m_stream_index++;
        }
        avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        out_stream->codecpar->codec_tag = 0;
    }
    if (avio_open(&m_outAVFormatContext->pb, out_url.c_str(), AVIO_FLAG_WRITE) < 0) {
        goto __ERR;
    }
    //write head info of file. Tips: mp4 need write foot info of file(av_write_trailer)
    if (avformat_write_header(m_outAVFormatContext, nullptr) < 0) {
        goto __ERR;
    }
    m_packet = av_packet_alloc();
    while (av_read_frame(m_inAVFormatContext, m_packet) >= 0) {
        AVStream *in_stream = m_inAVFormatContext->streams[m_packet->stream_index];
        AVStream *out_stream = m_outAVFormatContext->streams[m_packet->stream_index];
        {
            //stream_index
            if (m_packet->stream_index >= m_stream_mapping_size ||
                m_stream_mapping[m_packet->stream_index] < 0) {
                av_packet_unref(m_packet);
                continue;
            }
            m_packet->stream_index = m_stream_mapping[m_packet->stream_index];
        }
        m_packet->dts = av_rescale_q_rnd(m_packet->dts,
                                         in_stream->time_base,
                                         out_stream->time_base,
                                         AV_ROUND_NEAR_INF);
        m_packet->pts = av_rescale_q_rnd(m_packet->pts,
                                         in_stream->time_base,
                                         out_stream->time_base,
                                         AV_ROUND_NEAR_INF);
        m_packet->duration = av_rescale_q(m_packet->duration,
                                          in_stream->time_base,
                                          out_stream->time_base);
        m_packet->pos = -1;
        if (av_interleaved_write_frame(m_outAVFormatContext, m_packet) < 0) {
            goto __ERR;
        }
        av_packet_unref(m_packet);
    }
    //write foot info of file, exp mp4.
    av_write_trailer(m_outAVFormatContext);
    repack_free();
    return 0;
    __ERR:
    {
        LOGE("__ERR__ERR__ERR__ERR__ERR__repack_fmt");
        repack_free();
        return -1;
    }
}

void repack::repack_free() {
    if (m_packet) {
        av_packet_free(&m_packet);
    }
    if (m_inAVFormatContext) {
        avformat_free_context(m_inAVFormatContext);
    }
    if (m_outAVFormatContext) {
        avformat_free_context(m_outAVFormatContext);
    }
    if (m_stream_mapping) {
        av_freep(&m_stream_mapping);
    }
}

