//
// Created by 86193 on 2023/3/24.
//
#include "recode.h"

int recode::recode_codec(const std::string &in_url,
                         const std::string &out_url,
                         const AVConfig &out_config) {
    int err = -1;
    int line;
    {//open avformat
        //in
        m_in_avformat_ctx = avformat_alloc_context();
        if ((err = avformat_open_input(&m_in_avformat_ctx,
                                       in_url.c_str(),
                                       nullptr,
                                       nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if ((err = avformat_find_stream_info(m_in_avformat_ctx, nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        //out
        m_out_avformat_ctx = avformat_alloc_context();
        if ((err = avformat_alloc_output_context2(&m_out_avformat_ctx,
                                                  nullptr,
                                                  nullptr,
                                                  out_url.c_str())) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if ((err = avio_open(&m_out_avformat_ctx->pb, out_url.c_str(), AVIO_FLAG_WRITE)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
    }
    {//open avcodec
        //in
        m_av_stream_models = new AVStreamModel *[m_in_avformat_ctx->nb_streams];
        for (size_t i = 0; i < m_in_avformat_ctx->nb_streams; i++) {
            AVStream *in_stream = m_in_avformat_ctx->streams[i];
            if (in_stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
                in_stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
                continue;
            } else {
                m_av_stream_models_size++;
            }
            auto *model = new AVStreamModel();
            AVCodec *codec = (AVCodec*)avcodec_find_decoder(in_stream->codecpar->codec_id);
            if (codec == nullptr) {
                line = __LINE__;
                goto __ERR;
            }
            model->in_av_decode_ctx = avcodec_alloc_context3(codec);
            if (model->in_av_decode_ctx == nullptr) {
                line = __LINE__;
                goto __ERR;
            }
            if ((err = avcodec_parameters_to_context(model->in_av_decode_ctx,
                                                     in_stream->codecpar)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            if ((err = avcodec_open2(model->in_av_decode_ctx,
                                     codec,
                                     nullptr) < 0)) {
                line = __LINE__;
                goto __ERR;
            }
            model->codec_type = in_stream->codecpar->codec_type;
            model->stream_index = (int) i;
            m_av_stream_models[i] = model;
        }
        //out
        for (size_t i = 0; i < m_av_stream_models_size; i++) {
            AVStreamModel *model = m_av_stream_models[i];
            AVStream *in_stream = m_in_avformat_ctx->streams[model->stream_index];
            AVCodec *out_av_encoder = (AVCodec*)avcodec_find_encoder(
                    model->codec_type == AVMEDIA_TYPE_VIDEO ?
                    (out_config.AVVideoCodecID == AV_CODEC_ID_NONE ?
                     m_out_avformat_ctx->oformat->video_codec :
                     out_config.AVVideoCodecID) :
                    (out_config.AVAudioCodecID == AV_CODEC_ID_NONE ?
                     m_out_avformat_ctx->oformat->audio_codec :
                     out_config.AVAudioCodecID));
            if (out_av_encoder == nullptr) {
                line = __LINE__;
                goto __ERR;
            }
            AVCodecContext *out_av_encoder_ctx = avcodec_alloc_context3(out_av_encoder);
            if (out_av_encoder_ctx == nullptr) {
                line = __LINE__;
                goto __ERR;
            }
            LOGE("lib_x264 / fdk_aac, not build for current ffmpeg.so");
            LOGE("mpeg_encoder: %s", avcodec_get_name(out_av_encoder->id));
            if (model->codec_type == AVMEDIA_TYPE_VIDEO) {
                out_av_encoder_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
                out_av_encoder_ctx->codec_id = out_av_encoder->id;
                out_av_encoder_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
                /*out_av_encoder_ctx->width = in_stream->codecpar->width;
                out_av_encoder_ctx->height = in_stream->codecpar->height;
                out_av_encoder_ctx->gop_size = 12;
                out_av_encoder_ctx->time_base = AVRational{1, 25};
                out_av_encoder_ctx->bit_rate = 4000 * 8;*/
                out_av_encoder_ctx->width = out_config.v_width == 0 ?
                                            in_stream->codecpar->width :
                                            out_config.v_width;
                out_av_encoder_ctx->height = out_config.v_height == 0 ?
                                             in_stream->codecpar->height :
                                             out_config.v_height;
                out_av_encoder_ctx->gop_size = out_config.v_gop_size == 0 ?
                                               model->in_av_decode_ctx->gop_size :
                                               out_config.v_gop_size;
                out_av_encoder_ctx->framerate = model->in_av_decode_ctx->framerate;
                if (av_q2d(out_av_encoder_ctx->framerate) == 0) {
                    out_av_encoder_ctx->framerate = AVRational{25, 1};
                }
                out_av_encoder_ctx->time_base = out_config.v_fps == 0 ?
                                                (av_q2d(out_av_encoder_ctx->time_base) == 0 ?
                                                 AVRational{1, out_av_encoder_ctx->framerate.num} :
                                                 model->in_av_decode_ctx->time_base) :
                                                AVRational{1, out_config.v_fps};
                out_av_encoder_ctx->bit_rate = out_config.v_bit_rate == 0 ?
                                               4000 * 8 :
                                               out_config.v_bit_rate;
            } else if (model->codec_type == AVMEDIA_TYPE_AUDIO) {
                out_av_encoder_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
                out_av_encoder_ctx->codec_id = out_av_encoder->id;
                out_av_encoder_ctx->sample_fmt = AV_SAMPLE_FMT_S16;
                out_av_encoder_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
                out_av_encoder_ctx->channels = 2;
                out_av_encoder_ctx->sample_rate = 44100;
                out_av_encoder_ctx->time_base = (AVRational) {1, 44100};
                out_av_encoder_ctx->bit_rate = 64000;
                /*out_av_encoder_ctx->channel_layout = out_config.a_ch_layout == 0 ?
                                                     model->in_av_decode_ctx->channel_layout :
                                                     out_config.a_ch_layout;
                out_av_encoder_ctx->channels = out_config.a_ch_layout == 0 ?
                                               model->in_av_decode_ctx->channels :
                                               av_get_channel_layout_nb_channels(
                                                       out_config.a_ch_layout);
                out_av_encoder_ctx->sample_rate = out_config.a_sample_rate == 0 ?
                                                  model->in_av_decode_ctx->sample_rate :
                                                  out_config.a_sample_rate;
                out_av_encoder_ctx->time_base = (AVRational) {1, out_av_encoder_ctx->sample_rate};
                out_av_encoder_ctx->bit_rate = out_config.a_bit_rate == 0 ?
                                               96000 :
                                               out_config.a_bit_rate;*/
            }
            /*if (m_out_avformat_ctx->flags & AVFMT_GLOBALHEADER) {
                //Some formats want stream headers to be separate
                out_av_encoder_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }*/
            if ((err = avcodec_open2(out_av_encoder_ctx, out_av_encoder, nullptr)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            AVStream *out_stream = avformat_new_stream(m_out_avformat_ctx, nullptr);
            if (out_stream == nullptr) {
                line = __LINE__;
                goto __ERR;
            }
            out_stream->id = (int) m_out_avformat_ctx->nb_streams - 1;
            out_stream->time_base = out_av_encoder_ctx->time_base;
            if ((err = avcodec_parameters_from_context(out_stream->codecpar,
                                                       out_av_encoder_ctx)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            model->out_av_decode_ctx = out_av_encoder_ctx;
        }
    }
    {//write header
        if ((err = avformat_write_header(m_out_avformat_ctx, nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
    }
    {//transcode
        m_packet = av_packet_alloc();
        if (m_packet == nullptr) {
            line = __LINE__;
            goto __ERR;
        }
        m_frame = av_frame_alloc();
        if (m_frame == nullptr) {
            line = __LINE__;
            goto __ERR;
        }
        while (av_read_frame(m_in_avformat_ctx, m_packet) >= 0) {
            AVStreamModel *model = m_av_stream_models[m_packet->stream_index];
            if (model == nullptr) {
                continue;
            }
            if ((err = avcodec_send_packet(model->in_av_decode_ctx, m_packet)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            while (avcodec_receive_frame(model->in_av_decode_ctx, m_frame) >= 0) {
                AVFrame *dst_frame = av_frame_alloc();
                if (model->codec_type == AVMEDIA_TYPE_VIDEO) {
                    /*int align = 1;
                    int buff_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                                             model->av_decode_ctx->width,
                                                             model->av_decode_ctx->height,
                                                             align);
                    auto *buff = (uint8_t *) av_malloc(buff_size * sizeof(uint8_t));
                    av_image_fill_arrays(frame->data,
                                         frame->linesize,
                                         buff,
                                         AV_PIX_FMT_YUV420P,
                                         model->av_decode_ctx->width,
                                         model->av_decode_ctx->height,
                                         align);
                    av_free(buff);*/
                    dst_frame->format = model->out_av_decode_ctx->pix_fmt;
                    dst_frame->width = model->out_av_decode_ctx->width;
                    dst_frame->height = model->out_av_decode_ctx->height;
                    if (model->in_sws_ctx == nullptr) {
                        model->in_sws_ctx = sws_getContext(model->in_av_decode_ctx->width,
                                                           model->in_av_decode_ctx->height,
                                                           model->in_av_decode_ctx->pix_fmt,
                                                           dst_frame->width,
                                                           dst_frame->height,
                                                           AVPixelFormat(dst_frame->format),
                                                           SWS_FAST_BILINEAR,
                                                           nullptr,
                                                           nullptr,
                                                           nullptr);
                    }
                    dst_frame->pts = model->next_pts;
                    model->next_pts++;
                    av_frame_get_buffer(dst_frame, 0);//malloc frame->buffer
                    sws_scale(model->in_sws_ctx,
                              m_frame->data,
                              m_frame->linesize,
                              0,
                              m_frame->height,
                              dst_frame->data,
                              dst_frame->linesize);
                } else if (model->codec_type == AVMEDIA_TYPE_AUDIO) {
                    /*int buff_size = 0;
                    auto **buff = (uint8_t **) av_malloc(buff_size * sizeof(uint8_t));
                    int dst_line_size = 0;
                    av_samples_alloc_array_and_samples(&buff,
                                                       &dst_line_size,
                                                       av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO),
                                                       (int) nb_samples,
                                                       AV_SAMPLE_FMT_S16,
                                                       0);*/
                    dst_frame->format = model->out_av_decode_ctx->sample_fmt;//Cross=L R L R
                    dst_frame->channel_layout = model->out_av_decode_ctx->channel_layout;//L&R
                    dst_frame->channels = model->out_av_decode_ctx->channels;
                    dst_frame->sample_rate = model->out_av_decode_ctx->sample_rate;
                    if (model->in_swr_ctx == nullptr) {
                        model->in_swr_ctx = swr_alloc();
                        swr_alloc_set_opts(model->in_swr_ctx,
                                           (int64_t) dst_frame->channel_layout,
                                           AVSampleFormat(dst_frame->format),
                                           dst_frame->sample_rate,
                                           (int64_t) model->in_av_decode_ctx->channel_layout,
                                           model->in_av_decode_ctx->sample_fmt,
                                           model->in_av_decode_ctx->sample_rate,
                                           0,
                                           nullptr);
                        swr_init(model->in_swr_ctx);
                    }
                    //swr_get_delay: (if set sample_rate) get nb_samples last time has not done.
                    int64_t delay_nb_samples = swr_get_delay(model->in_swr_ctx,
                                                             m_frame->sample_rate);
                    dst_frame->nb_samples = (int) av_rescale_rnd(
                            m_frame->nb_samples + delay_nb_samples,
                            dst_frame->sample_rate,
                            m_frame->sample_rate,
                            AV_ROUND_UP
                    );
                    dst_frame->pts = model->next_pts;
                    model->next_pts += dst_frame->nb_samples;
                    av_frame_get_buffer(dst_frame, 0);//malloc frame->buffer
                    swr_convert(model->in_swr_ctx,
                                dst_frame->data,
                                dst_frame->nb_samples,
                                (const uint8_t **) m_frame->data,
                                m_frame->nb_samples);
                    LOGE("AUDIO, format=%d, pts=%ld, sample_rate=%d, nb_samples=%d",
                         dst_frame->format,
                         dst_frame->pts,
                         dst_frame->sample_rate,
                         dst_frame->nb_samples);
                }
                if ((err = av_frame_make_writable(dst_frame)) < 0) {
                    line = __LINE__;
                    goto __ERR;
                } else {
                    if ((err = avcodec_send_frame(model->out_av_decode_ctx, dst_frame)) < 0) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    AVPacket *dst_packet = av_packet_alloc();
                    while (avcodec_receive_packet(model->out_av_decode_ctx, dst_packet) >= 0) {
                        dst_packet->stream_index = model->stream_index;
                        av_packet_rescale_ts(dst_packet, model->out_av_decode_ctx->time_base,
                                             m_out_avformat_ctx->streams[model->stream_index]->time_base);
                        if ((err = av_interleaved_write_frame(m_out_avformat_ctx,
                                                              dst_packet)) < 0) {
                            line = __LINE__;
                            goto __ERR;
                        }
                        av_packet_unref(dst_packet);
                    }
                    av_frame_unref(dst_frame);
                    av_packet_free(&dst_packet);
                    av_frame_free(&dst_frame);
                }
                av_frame_unref(m_frame);
            }
            av_packet_unref(m_packet);
        }
    }
    {//write footer
        av_write_trailer(m_out_avformat_ctx);
    }
    {//free
        recode_free();
        return 0;
    }
    __ERR:
    {
        LOGE("__ERR__ERR__ERR__ERR__ERR__recode_codec(), line=%d, err=%d", line, err);
        recode_free();
        return err;
    }
}

void recode::recode_free() {
    if (m_frame) {
        av_frame_free(&m_frame);
    }
    if (m_packet) {
        av_packet_free(&m_packet);
    }
    if (m_av_stream_models) {
        for (size_t i = 0; i < m_av_stream_models_size; i++) {
            AVStreamModel *model = m_av_stream_models[i];
            if (model->in_av_decode_ctx) {
                avcodec_free_context(&model->in_av_decode_ctx);
            }
            if (model->in_sws_ctx) {
                sws_freeContext(model->in_sws_ctx);
            }
            if (model->in_swr_ctx) {
                swr_free(&model->in_swr_ctx);
            }
            if (model->out_av_decode_ctx) {
                avcodec_free_context(&model->out_av_decode_ctx);
            }
        }
    }
    if (m_in_avformat_ctx) {
        avformat_free_context(m_in_avformat_ctx);
    }
    if (m_out_avformat_ctx) {
        avformat_free_context(m_out_avformat_ctx);
    }
}

