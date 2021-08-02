#include "MediaRecord.h"

extern "C" {

void MediaRecord::onSource(char *urlOut, MediaConfig config) {
    m_UrlOut = urlOut;
    m_Config = config;
    int width = m_Config.width;
    int height = m_Config.height;
    m_Config.width = height;
    m_Config.height = width;
}

void MediaRecord::onPrepare() {
    int result = 0;
    //1.打开封装上下文 AVFormatContext -> 打开封装工具 AVOutputFormat
    avformat_alloc_output_context2(&m_AVFormatContext, NULL, NULL, m_UrlOut);
    if (!m_AVFormatContext) {
        LOGCATE("MediaRecorder::avformat_alloc_output_context2 error , try using MPEG.");
        avformat_alloc_output_context2(&m_AVFormatContext, NULL, "mpeg", m_UrlOut);
    }
    m_AVOutputFormat = const_cast<AVOutputFormat *>(m_AVFormatContext->oformat);
    if ((m_AVOutputFormat->flags & AVFMT_NOFILE)) {
        LOGCATE("MediaRecorder::AVOutputFormat->flags & AVFMT_NOFILE");
        return;
    }
    //3.打开io上下文 AVIOContext -> 打开io流
    result = avio_open(&m_AVFormatContext->pb, m_UrlOut, AVIO_FLAG_WRITE);
    if (result < 0) {
        LOGCATE("MediaRecorder::avio_open error '%s': %s", m_UrlOut, av_err2str(result));
        return;
    }
    if (m_AVOutputFormat->video_codec != AV_CODEC_ID_NONE) {
        result = initStream(m_AVFormatContext, &m_VideoStream);
    }
    if (result < 0) {
        LOGCATE("MediaRecorder::initStream video error");
        return;
    }
    if (m_AVOutputFormat->audio_codec != AV_CODEC_ID_NONE) {
        result = initStream(m_AVFormatContext, &m_AudioStream);
    }
    if (result < 0) {
        LOGCATE("MediaRecorder::initStream audio error");
        return;
    }
    //4.查找对应视频编码器->打开编码器上下文->打开编码器 (使用默认编码格式,codecId: AV_CODEC_ID_H264 etc.)
    result = initCodec(m_AVOutputFormat->video_codec, m_VideoCodec, &m_VideoStream);
    if (result < 0) {
        LOGCATE("MediaRecorder::initCodec video error");
        return;
    }
    result = initCodec(m_AVOutputFormat->audio_codec, m_AudioCodec, &m_AudioStream);
    if (result < 0) {
        LOGCATE("MediaRecorder::initCodec audio error");
        return;
    }
    //5.写入头文件
    AVDictionary *opt = 0;
    if (m_AVOutputFormat->video_codec == AV_CODEC_ID_H264) {
        av_dict_set_int(&opt, "video_track_timescale", 25, 0);
        av_dict_set(&opt, "preset", "slow", 0);
        av_dict_set(&opt, "tune", "zerolatency", 0);
    }
    result = avformat_write_header(m_AVFormatContext, nullptr);
    if (result < 0) {
        LOGCATE("MediaRecorder::onPrepare avformat_write_header error: %s", av_err2str(result));
        return;
    }
    //6.开启循环线程
    if (result >= 0) {
        if (m_Thread == nullptr) m_Thread = new std::thread(onRunAsy, this);
        LOGCATE("MediaRecord::code loop start");
    }
    //打印关于输入或输出格式的详细信息
    av_dump_format(m_AVFormatContext, 0, m_UrlOut, 1);
}

void MediaRecord::onBufferVideo(int format, int width, int height, uint8_t *data) {
    if (m_Interrupt) return;
    if (data == nullptr) return;
    int w = height;
    int h = width;
    VideoFrame *frame = new VideoFrame();
    frame->image = PixImageUtils::pix_image_get(format, width, height, data);
//    std::lock_guard<std::mutex> lock(m_Mutex);
    m_VideoQueue.push(frame);
}

void MediaRecord::onBufferAudio(AudioFrame *input) {
    if (m_Interrupt) return;
    AudioFrame *frame = new AudioFrame(input->data, input->dataSize);
//    std::lock_guard<std::mutex> lock(m_Mutex);
    m_AudioQueue.push(frame);
}

void MediaRecord::onRelease() {
    if (m_Thread == nullptr) return;
    LOGCATE("MediaRecorder::code loop join");
    m_Interrupt = true;
    m_Thread->join();
    delete m_Thread;
    m_Thread = nullptr;
    int ret = av_write_trailer(m_AVFormatContext);
    LOGCATE("MediaRecorder::StopRecord while av_write_trailer %s", av_err2str(ret));
    release();
}

int MediaRecord::initStream(AVFormatContext *fmtCtx, AVOutputStream *stream) {
    stream->m_Stream = avformat_new_stream(fmtCtx, NULL);
    if (!stream->m_Stream) {
        LOGCATE("MediaRecorder::initStream avformat_new_stream error");
        return -1;
    }
    stream->m_Stream->id = fmtCtx->nb_streams - 1;
    return 1;
}

int MediaRecord::initCodec(AVCodecID codecId, AVCodec *codec, AVOutputStream *stream) {
    codec = const_cast<AVCodec *>(avcodec_find_encoder(codecId));
    if (!codec) {
        LOGCATE("MediaRecorder::avcodec_find_encoder error '%s'", avcodec_get_name(codecId));
        return -1;
    }
    AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        LOGCATE("MediaRecorder::avcodec_alloc_context3 error '%s'", avcodec_get_name(codecId));
        return -1;
    }
    if (codec->type == AVMEDIA_TYPE_VIDEO) {
        codecCtx->codec_id = codecId;
        codecCtx->bit_rate = m_Config.vBitRate;
        codecCtx->width = m_Config.width;
        codecCtx->height = m_Config.height;
        codecCtx->gop_size = 12;
        codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        if (codecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            codecCtx->max_b_frames = 2;
        }
        if (codecCtx->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            codecCtx->mb_decision = 2;
        }
        codecCtx->time_base = (AVRational) {1, m_Config.fps};
        stream->m_Stream->time_base = (AVRational) {1, m_Config.fps};
    } else if (codec->type == AVMEDIA_TYPE_AUDIO) {
        codecCtx->sample_fmt = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        codecCtx->bit_rate = 96000;
        codecCtx->sample_rate = m_Config.aSampleRate;
        codecCtx->channel_layout = m_Config.aChannel;
        codecCtx->channels = av_get_channel_layout_nb_channels(m_Config.aChannel);
        stream->m_Stream->time_base = (AVRational) {1, m_Config.aSampleRate};
    }
    //Some formats want stream headers to be separate
    if (m_AVOutputFormat->flags & AVFMT_GLOBALHEADER) {
        codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    int result = avcodec_open2(codecCtx, codec, nullptr);
    if (result < 0) {
        LOGCATE("MediaRecorder::avcodec_open2 error: %s", av_err2str(result));
        return -1;
    }
    stream->m_CodecCtx = codecCtx;
    //?????????????????????????????????????????????????????????????????????????????????
    //Muxer + SWS + SWR
    if (codec->type == AVMEDIA_TYPE_VIDEO) {
        stream->m_Frame = av_frame_alloc();
        stream->m_Frame->format = codecCtx->pix_fmt;
        stream->m_Frame->width = codecCtx->width;
        stream->m_Frame->height = codecCtx->height;
        result = av_frame_get_buffer(stream->m_Frame, 1);
        if (result < 0) {
            LOGCATE("MediaRecorder::av_frame_get_buffer video error");
            return -1;
        }
        stream->m_TmpFrame = av_frame_alloc();
        //copy the stream parameters to the muxer
        result = avcodec_parameters_from_context(stream->m_Stream->codecpar, codecCtx);
        if (result < 0) {
            LOGCATE("MediaRecorder::avcodec_parameters_from_context video error");
            return -1;
        }
    } else if (codec->type == AVMEDIA_TYPE_AUDIO) {
        int nb_samples;
        if (codecCtx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) {
            nb_samples = 10000;
        } else {
            nb_samples = codecCtx->frame_size;
        }
        stream->m_Frame = av_frame_alloc();
        stream->m_Frame->format = codecCtx->sample_fmt;
        stream->m_Frame->channel_layout = codecCtx->channel_layout;
        stream->m_Frame->sample_rate = codecCtx->sample_rate;
        stream->m_Frame->nb_samples = nb_samples;
        if (nb_samples) {
            result = av_frame_get_buffer(stream->m_Frame, 0);
            if (result < 0) {
                LOGCATE("MediaRecorder::av_frame_get_buffer audio error");
                return -1;
            }
        }
        stream->m_TmpFrame = av_frame_alloc();
        //copy the stream parameters to the muxer
        result = avcodec_parameters_from_context(stream->m_Stream->codecpar, codecCtx);
        if (result < 0) {
            LOGCATE("MediaRecorder::avcodec_parameters_from_context audio error");
            return -1;
        }
        //create reSample context
        stream->m_SwrCtx = swr_alloc();
        if (!stream->m_SwrCtx) {
            LOGCATE("MediaRecorder::swr_alloc error");
            return -1;
        }
        av_opt_set_int(stream->m_SwrCtx, "in_channel_count", codecCtx->channels, 0);
        av_opt_set_int(stream->m_SwrCtx, "in_sample_rate", codecCtx->sample_rate, 0);
        av_opt_set_sample_fmt(stream->m_SwrCtx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
        av_opt_set_int(stream->m_SwrCtx, "out_channel_count", codecCtx->channels, 0);
        av_opt_set_int(stream->m_SwrCtx, "out_sample_rate", codecCtx->sample_rate, 0);
        av_opt_set_sample_fmt(stream->m_SwrCtx, "out_sample_fmt", codecCtx->sample_fmt, 0);
        if ((result = swr_init(stream->m_SwrCtx)) < 0) {
            LOGCATE("MediaRecorder::swr_init error");
            return -1;
        }
    }
    return result;
}

int MediaRecord::codeVFrame(AVOutputStream *stream) {
    while (m_VideoQueue.empty() && !m_Interrupt) {
        usleep(10 * 1000);
    }
    std::unique_lock<std::mutex> lock(m_Mutex);
    int result = 0;
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = stream->m_TmpFrame;
    VideoFrame *vFrame = m_VideoQueue.front();
    m_VideoQueue.pop();
    if (vFrame) {
        frame->data[0] = vFrame->image->plane[0];
        frame->data[1] = vFrame->image->plane[1];
        frame->data[2] = vFrame->image->plane[2];
        frame->linesize[0] = vFrame->image->pLineSize[0];
        frame->linesize[1] = vFrame->image->pLineSize[1];
        frame->linesize[2] = vFrame->image->pLineSize[2];
        frame->width = vFrame->image->width;
        frame->height = vFrame->image->height;
    }
    if (m_VideoQueue.empty() && m_Interrupt) {
        frame = nullptr;
        result = 1;
        goto EXIT;
    }
    lock.unlock();
    if (frame) {
        if (!stream->m_SwsCtx) {
            AVPixelFormat ftm = AV_PIX_FMT_RGBA;
            switch (vFrame->image->format) {
                case IMAGE_FORMAT_RGBA:
                    ftm = AV_PIX_FMT_RGBA;
                    break;
                case IMAGE_FORMAT_NV21:
                    ftm = AV_PIX_FMT_NV21;
                    break;
                case IMAGE_FORMAT_NV12:
                    ftm = AV_PIX_FMT_NV12;
                    break;
                case IMAGE_FORMAT_YUV420P:
                    ftm = AV_PIX_FMT_YUV420P;
                    break;
                default:
                    LOGCATE("MediaRecorder::not support this format");
                    break;
            }
            stream->m_SwsCtx = sws_getContext(stream->m_CodecCtx->width,
                                              stream->m_CodecCtx->height,
                                              ftm,
                                              stream->m_CodecCtx->width,
                                              stream->m_CodecCtx->height,
                                              stream->m_CodecCtx->pix_fmt,
                                              SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
            if (!stream->m_SwsCtx) {
                LOGCATE("MediaRecorder::sws_getContext Could not initialize the conversion context\n");
                result = 0;
                goto EXIT;
            }
        }
        if ((result = av_frame_make_writable(stream->m_Frame)) < 0) {
            LOGCATE("MediaRecorder::av_frame_make_writable video ret=%s", av_err2str(result));
            result = 0;
            goto EXIT;
        }
        result = sws_scale(stream->m_SwsCtx, frame->data, frame->linesize,
                           0, stream->m_CodecCtx->height,
                           stream->m_Frame->data, stream->m_Frame->linesize);
        if (result < 0) {
            LOGCATE("MediaRecorder::sws_scale video ret=%s", av_err2str(result));
            result = 0;
            goto EXIT;
        }
        if ((result = av_frame_make_writable(stream->m_Frame)) < 0) {
            result = 0;
            goto EXIT;
        }
        stream->m_Frame->pts = stream->m_NextPts++;
        frame = stream->m_Frame;
        //code frame
        result = avcodec_send_frame(stream->m_CodecCtx, frame);
        if (result == AVERROR_EOF) {
            LOGCATE("MediaRecorder::avcodec_send_frame video EOF ret=%s", av_err2str(result));
            result = 1;
            goto EXIT;
        } else if (result < 0) {
            LOGCATE("MediaRecorder::avcodec_send_frame video error ret=%s", av_err2str(result));
            result = 0;
            goto EXIT;
        }
        while (!result) {
            result = avcodec_receive_packet(stream->m_CodecCtx, packet);
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
//                LOGCATE("MediaRecorder::avcodec_receive_packet AVERROR ret=%s", av_err2str(result));
                result = 0;
                goto EXIT;
            } else if (result < 0) {
                LOGCATE("MediaRecorder::avcodec_receive_packet error ret=%s", av_err2str(result));
                result = 0;
                goto EXIT;
            }
            //Write the compressed frame to the media file
            av_packet_rescale_ts(packet, stream->m_CodecCtx->time_base,
                                 stream->m_Stream->time_base);
            packet->stream_index = stream->m_Stream->index;
            result = av_interleaved_write_frame(m_AVFormatContext, packet);
            av_packet_unref(packet);
            if (result < 0) {
                LOGCATE("MediaRecorder::av_interleaved_write_frame error: %s", av_err2str(result));
                result = 0;
                goto EXIT;
            }
        }
    }
    EXIT:
    if (vFrame) {
        PixImageUtils::pix_image_free(vFrame->image);
        delete vFrame;
    }
    return result;
}

int MediaRecord::codeAFrame(AVOutputStream *stream) {
    while (m_AudioQueue.empty() && !m_Interrupt) {
        usleep(10 * 1000);
    }
    std::unique_lock<std::mutex> lock(m_Mutex);
    int result = 0;
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    frame = stream->m_TmpFrame;
    if (m_AudioQueue.empty() && !m_VideoQueue.empty() && m_Interrupt) {
        return -100;
    }
    AudioFrame *aFrame = m_AudioQueue.front();
    m_AudioQueue.pop();
    if (aFrame && aFrame != 0) {
        frame->data[0] = aFrame->data;
        frame->nb_samples = aFrame->dataSize / 4;
        frame->pts = stream->m_NextPts;
        stream->m_NextPts += frame->nb_samples;
    }
    if (m_AudioQueue.empty() && m_Interrupt) {
        frame = nullptr;
        result = 1;
        goto EXIT;
    }
    lock.unlock();
    if (frame) {
        //get dst_nb_samples AND re_sample
        int dst_nb_samples = av_rescale_rnd(
                swr_get_delay(stream->m_SwrCtx, stream->m_CodecCtx->sample_rate) +
                frame->nb_samples,
                stream->m_CodecCtx->sample_rate,
                stream->m_CodecCtx->sample_rate,
                AV_ROUND_UP);
        av_assert0(dst_nb_samples == frame->nb_samples);
        if ((result = av_frame_make_writable(stream->m_Frame)) < 0) {
            LOGCATE("MediaRecorder::av_frame_make_writable audio error");
            result = 0;
            goto EXIT;
        }
        result = swr_convert(stream->m_SwrCtx, stream->m_Frame->data, dst_nb_samples,
                             (const uint8_t **) frame->data, frame->nb_samples);
        if (result < 0) {
            LOGCATE("MediaRecorder::swr_convert audio error");
            result = 0;
            goto EXIT;
        }
        frame = stream->m_Frame;
        frame->pts = av_rescale_q(stream->m_SamplesCount,
                                  (AVRational) {1, stream->m_CodecCtx->sample_rate},
                                  stream->m_CodecCtx->time_base);
        stream->m_SamplesCount += dst_nb_samples;
        //code frame
        result = avcodec_send_frame(stream->m_CodecCtx, frame);
        if (result == AVERROR_EOF) {
            result = 1;
            goto EXIT;
        } else if (result < 0) {
            LOGCATE("MediaRecorder::avcodec_send_frame error audio ret=%s", av_err2str(result));
            result = 0;
            goto EXIT;
        }
        while (!result) {
            result = avcodec_receive_packet(stream->m_CodecCtx, packet);
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                result = 0;
                goto EXIT;
            } else if (result < 0) {
                LOGCATE("MediaRecorder::avcodec_receive_packet audio ret=%s", av_err2str(result));
                result = 0;
                goto EXIT;
            }
            av_packet_rescale_ts(packet, stream->m_CodecCtx->time_base,
                                 stream->m_Stream->time_base);
            packet->stream_index = stream->m_Stream->index;
            result = av_interleaved_write_frame(m_AVFormatContext, packet);
            if (result < 0) {
                LOGCATE("MediaRecorder::av_interleaved_write_frame error: %s", av_err2str(result));
                result = 0;
                goto EXIT;
            }
        }
    }
    EXIT:
    if (aFrame && aFrame != 0) delete aFrame;
    return result;
}

void MediaRecord::release() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    while (!m_VideoQueue.empty()) {
        VideoFrame *frame = m_VideoQueue.front();
        m_VideoQueue.pop();
        PixImageUtils::pix_image_free(frame->image);
        delete frame;
    }
    while (!m_AudioQueue.empty()) {
        AudioFrame *frame = m_AudioQueue.front();
        m_AudioQueue.pop();
        delete frame;
    }
    lock.unlock();
    //Close each codec
    avcodec_free_context(&m_VideoStream.m_CodecCtx);
    av_frame_free(&m_VideoStream.m_Frame);
    sws_freeContext(m_VideoStream.m_SwsCtx);
    if (!m_VideoStream.m_TmpFrame) {
        av_free(m_VideoStream.m_TmpFrame);
        m_VideoStream.m_TmpFrame = nullptr;
    }
    avcodec_free_context(&m_AudioStream.m_CodecCtx);
    av_frame_free(&m_AudioStream.m_Frame);
    swr_free(&m_AudioStream.m_SwrCtx);
    if (!m_AudioStream.m_TmpFrame) {
        av_free(m_AudioStream.m_TmpFrame);
        m_AudioStream.m_TmpFrame = nullptr;
    }
    if (!(m_AVOutputFormat->flags & AVFMT_NOFILE)) {
        avio_closep(&m_AVFormatContext->pb);
    }
    avformat_free_context(m_AVFormatContext);
}

void MediaRecord::onRunAsy(MediaRecord *p) {
    AVOutputStream *vStream = &p->m_VideoStream;
    AVOutputStream *aStream = &p->m_AudioStream;
    while (!vStream->m_EncodeEnd && !aStream->m_EncodeEnd) {
        double vStamp = vStream->m_NextPts * av_q2d(vStream->m_CodecCtx->time_base);
        double aStamp = aStream->m_NextPts * av_q2d(aStream->m_CodecCtx->time_base);
        if (av_compare_ts(vStream->m_NextPts, vStream->m_CodecCtx->time_base,
                          aStream->m_NextPts, aStream->m_CodecCtx->time_base) <= 0) {
            //视频和音频时间戳对齐，人对于声音比较敏感，防止出现视频声音播放结束画面还没结束的情况
            if (aStamp <= vStamp && aStream->m_EncodeEnd) {
                vStream->m_EncodeEnd = 1;
                aStream->m_EncodeEnd = 1;
            }
            vStream->m_EncodeEnd = p->codeVFrame(vStream);
        } else {
            if (aStream->m_EncodeEnd == -100) {//停止录制时，消耗剩余帧
                vStream->m_EncodeEnd = p->codeVFrame(vStream);
            } else {
                aStream->m_EncodeEnd = p->codeAFrame(aStream);
            }
        }
    }
}
}





