#include "MediaRecord.h"

extern "C" {

void MediaRecord::onSource(char *urlOut, MediaConfig config) {
    m_UrlOut = urlOut;
    m_Config = config;
}

void MediaRecord::onPrepare() {
    int result = 0;
    //AVFormat Context init
    avformat_alloc_output_context2(&m_AVFormatContext, NULL, NULL, m_UrlOut);
    if (!m_AVFormatContext) {
        LOGCATE("MediaRecorder::onPrepare avformat_alloc_output_context2 error , try using MPEG.\n");
        avformat_alloc_output_context2(&m_AVFormatContext, NULL, "mpeg", m_UrlOut);
    }
    if (!m_AVFormatContext) {
        result = -1;
        return;
    }
    //AVOutputFormat Context init
    m_AVOutputFormat = const_cast<AVOutputFormat *>(m_AVFormatContext->oformat);
    //Stream init
    if (m_AVOutputFormat->video_codec != AV_CODEC_ID_NONE) {
        result = initStream(m_VideoCodec, m_AVOutputFormat->video_codec, &m_VideoStream);
    }
    if (m_AVOutputFormat->audio_codec != AV_CODEC_ID_NONE) {
        result = initStream(m_AudioCodec, m_AVOutputFormat->audio_codec, &m_AudioStream);
    }
    if (result < 0) return;
    av_dump_format(m_AVFormatContext, 0, m_UrlOut, 1);
    //open the output file, if needed
    if (!(m_AVOutputFormat->flags & AVFMT_NOFILE)) {
        result = avio_open(&m_AVFormatContext->pb, m_UrlOut, AVIO_FLAG_WRITE);
        if (result < 0) {
            LOGCATE("MediaRecorder::onPrepare Cant open '%s': %s", m_UrlOut, av_err2str(result));
            result = -1;
            return;
        }
    }
    // Write the stream header, if any
    result = avformat_write_header(m_AVFormatContext, nullptr);
    if (result < 0) {
        LOGCATE("MediaRecorder::onPrepare avformat_write_header error: %s", av_err2str(result));
        result = -1;
        return;
    }
    if (result >= 0) {
        if (m_Thread == nullptr) m_Thread = new std::thread(onRunAsy, this);
    }
}

void MediaRecord::onBufferVideo(VideoFrame *input) {
    if (m_Interrupt) return;
    VideoFrame *frame = new VideoFrame();
    frame->image = PixImageUtils::pix_image_get(input->image->format, input->image->width,
                                                input->image->height,
                                                input->image->pLineSize, input->image->plane);
    m_VideoQueue.push(frame);
}

void MediaRecord::onBufferAudio(AudioFrame *input) {
    if (m_Interrupt) return;
    AudioFrame *frame = new AudioFrame(input->data, input->dataSize);
    m_AudioQueue.push(frame);
}

void MediaRecord::onRelease() {
    m_Interrupt = true;
    if (m_Thread != nullptr) {
        m_Thread->join();
        delete m_Thread;
        m_Thread = nullptr;
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
        av_write_trailer(m_AVFormatContext);
        //Close each codec
        avcodec_free_context(&m_VideoStream.m_CodecCtx);
        av_frame_free(&m_VideoStream.m_Frame);
        sws_freeContext(m_VideoStream.m_SwsCtx);
        swr_free(&m_VideoStream.m_SwrCtx);
        if (!m_VideoStream.m_TmpFrame) {
            av_free(m_VideoStream.m_TmpFrame);
            m_VideoStream.m_TmpFrame = nullptr;
        }
        avcodec_free_context(&m_AudioStream.m_CodecCtx);
        av_frame_free(&m_AudioStream.m_Frame);
        sws_freeContext(m_AudioStream.m_SwsCtx);
        swr_free(&m_AudioStream.m_SwrCtx);
        if (!m_AudioStream.m_TmpFrame) {
            av_free(m_AudioStream.m_TmpFrame);
            m_AudioStream.m_TmpFrame = nullptr;
        }
        if (!(m_AVOutputFormat->flags & AVFMT_NOFILE)) {
            avio_closep(&m_AVFormatContext->pb);// Close the output file
        }
        avformat_free_context(m_AVFormatContext);//free the stream
    }
}

int MediaRecord::initStream(AVCodec *avCodec, AVCodecID avCodecId, AVOutputStream *avStream) {
    //Codec init
    avCodec = const_cast<AVCodec *>(avcodec_find_decoder(avCodecId));
    if (!avCodec) {
        LOGCATE("MediaRecorder::initStream avcodec_find_decoder error videocodec");
        return -1;
    }
    //Stream init
    avStream->m_Stream = avformat_new_stream(m_AVFormatContext, NULL);
    if (!avStream->m_Stream) {
        LOGCATE("MediaRecorder::initStream avformat_new_stream error avStream");
        return -1;
    }
    avStream->m_Stream->id = m_AVFormatContext->nb_streams - 1;
    //Codec Context init
    AVCodecContext *codecCtx = avcodec_alloc_context3(avCodec);
    if (!codecCtx) {
        LOGCATE("MediaRecorder::initStream Could not alloc an encoding context");
        return -1;
    }
    avStream->m_CodecCtx = codecCtx;
    //info init
    if (avCodec->type == AVMEDIA_TYPE_AUDIO) {
        codecCtx->sample_fmt = avCodec->sample_fmts ? avCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        codecCtx->bit_rate = m_Config.aBitRate;
        codecCtx->sample_rate = m_Config.aSampleRate;
        codecCtx->channel_layout = m_Config.aChannel;
        codecCtx->channels = av_get_channel_layout_nb_channels(m_Config.aChannel);
        codecCtx->time_base = (AVRational) {1, m_Config.aSampleRate};
    } else if (avCodec->type == AVMEDIA_TYPE_VIDEO) {
        codecCtx->codec_id = avCodecId;
        codecCtx->bit_rate = m_Config.vBitRate;
        codecCtx->width = m_Config.width;
        codecCtx->height = m_Config.height;
        codecCtx->time_base = (AVRational) {1, m_Config.fps};
        codecCtx->gop_size = 12; //emit one intra frame every twelve frames at most
        codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        if (codecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            codecCtx->max_b_frames = 2;// just for testing, we also add B-frames
        }
        if (codecCtx->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            codecCtx->mb_decision = 2;//Needed to avoid using macroblocks in which some coeffs overflow(plane does not match the luma plane.)
        }
    }
    //Some formats want stream headers to be separate.
    if (m_AVFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    //codec init
    return initCodec(codecCtx, avCodec, avCodecId, avStream);
}

int MediaRecord::initCodec(AVCodecContext *codecCtx, AVCodec *avCodec, AVCodecID avCodecId,
                           AVOutputStream *avStream) {
    int result = avcodec_open2(codecCtx, avCodec, nullptr);
    if (result < 0) {
        LOGCATE("MediaRecorder::initCodec Could not open  codec: %s", av_err2str(result));
        return -1;
    }
    //frame init
    if (avCodec->type == AVMEDIA_TYPE_VIDEO) {
        //alloc video
        AVFrame *avFrame = av_frame_alloc();
        if (!avFrame) return -1;
        avFrame->format = codecCtx->pix_fmt;
        avFrame->width = codecCtx->width;
        avFrame->height = codecCtx->height;
        result = av_frame_get_buffer(avFrame, 1);
        if (result < 0) {
            LOGCATE("MediaRecorder::initCodec av_frame_get_buffer error");
            return -1;
        }
        avStream->m_Frame = avFrame;
        avStream->m_TmpFrame = av_frame_alloc();
        if (!avStream->m_Frame) {
            LOGCATE("MediaRecorder::initCodec av_frame_alloc video error");
            return -1;
        }
        //copy the stream parameters to the muxer
        result = avcodec_parameters_from_context(avStream->m_Stream->codecpar, codecCtx);
        if (result < 0) {
            LOGCATE("MediaRecorder::initCodec avcodec_parameters_from_context video error");
            return -1;
        }
    } else if (avCodec->type == AVMEDIA_TYPE_AUDIO) {
        int nb_samples = 0;
        if (codecCtx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) {
            nb_samples = 10000;
        } else {
            nb_samples = codecCtx->frame_size;
        }
        AVFrame *frame = av_frame_alloc();
        if (!frame) {
            LOGCATE("MediaRecorder::initCodec av_frame_alloc error audio frame");
            return -1;
        }
        frame->format = codecCtx->pix_fmt;
        frame->channel_layout = codecCtx->channel_layout;
        frame->sample_rate = codecCtx->sample_rate;
        frame->nb_samples = nb_samples;
        if (frame->nb_samples) {
            result = av_frame_get_buffer(frame, 0);
            if (result < 0) {
                LOGCATE("MediaRecorder::AllocAudioFrame Error allocating an audio buffer");
                return -1;
            }
        }
        avStream->m_TmpFrame = av_frame_alloc();
        //copy the stream parameters to the muxer
        result = avcodec_parameters_from_context(avStream->m_Stream->codecpar, codecCtx);
        if (result < 0) {
            LOGCATE("MediaRecorder::initCodec avcodec_parameters_from_context audio error");
            return -1;
        }
        //create resampler context
        avStream->m_SwrCtx = swr_alloc();
        if (!avStream->m_SwrCtx) {
            LOGCATE("MediaRecorder::initCodec Could not allocate resampler context");
            return -1;
        }
        //set options
        av_opt_set_int(avStream->m_SwrCtx, "in_channel_count", codecCtx->channels, 0);
        av_opt_set_int(avStream->m_SwrCtx, "in_sample_rate", codecCtx->sample_rate, 0);
        av_opt_set_sample_fmt(avStream->m_SwrCtx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
        av_opt_set_int(avStream->m_SwrCtx, "out_channel_count", codecCtx->channels, 0);
        av_opt_set_int(avStream->m_SwrCtx, "out_sample_rate", codecCtx->sample_rate, 0);
        av_opt_set_sample_fmt(avStream->m_SwrCtx, "out_sample_fmt", codecCtx->sample_fmt, 0);
        //initialize the resampling context
        if ((result = swr_init(avStream->m_SwrCtx)) < 0) {
            LOGCATE("MediaRecorder::initCodec Failed to initialize the resampling context");
            return -1;
        }
    }
    return result;
}

int MediaRecord::codeVideoFrame(AVOutputStream *stream) {
    int result = 0;
    while (m_VideoQueue.empty() && !m_Interrupt) {
        usleep(10 * 1000);
    }
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = stream->m_TmpFrame;
    VideoFrame *queueFrame = m_VideoQueue.front();
    m_VideoQueue.pop();
    if (queueFrame) {
        frame->data[0] = queueFrame->image->plane[0];
        frame->data[1] = queueFrame->image->plane[1];
        frame->data[2] = queueFrame->image->plane[2];
        frame->linesize[0] = queueFrame->image->pLineSize[0];
        frame->linesize[1] = queueFrame->image->pLineSize[1];
        frame->linesize[2] = queueFrame->image->pLineSize[2];
        frame->width = queueFrame->image->width;
        frame->height = queueFrame->image->height;
    }
    if ((m_VideoQueue.empty() && m_Interrupt) || stream->m_EncodeEnd) frame = nullptr;
    if (frame) {
        if (av_frame_make_writable(stream->m_Frame) < 0) {
            result = 1;
            goto EXIT;
        }
        if (queueFrame->image->format != IMAGE_FORMAT_YUV420P &&
            queueFrame->image->format != IMAGE_FORMAT_NV21 &&
            queueFrame->image->format != IMAGE_FORMAT_NV12 &&
            queueFrame->image->format != IMAGE_FORMAT_RGBA) {
            AVPixelFormat ftm = AV_PIX_FMT_RGBA;
            switch (queueFrame->image->format) {
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
                    ftm = AV_PIX_FMT_RGBA;
                    break;
            }
            if (!stream->m_SwsCtx) {
                stream->m_SwsCtx = sws_getContext(stream->m_CodecCtx->width,
                                                  stream->m_CodecCtx->height,
                                                  ftm,
                                                  stream->m_CodecCtx->width,
                                                  stream->m_CodecCtx->height,
                                                  stream->m_CodecCtx->pix_fmt,
                                                  SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
            }
            if (!stream->m_SwsCtx) {
                LOGCATE("MediaRecorder::sws_getContext Could not initialize the conversion context\n");
                result = 1;
                goto EXIT;
            }
            sws_scale(stream->m_SwsCtx, (const uint8_t *const *) frame->data,
                      frame->linesize, 0, stream->m_CodecCtx->height, stream->m_Frame->data,
                      stream->m_Frame->linesize);
            frame = stream->m_Frame;
        }
        frame->pts = stream->m_NextPts++;
        //code frame
        result = avcodec_send_frame(stream->m_CodecCtx, frame);
        if (result == AVERROR_EOF) {
            result = 1;
            goto EXIT;
        } else if (result < 0) {
            LOGCATE("MediaRecorder::avcodec_send_frame error. ret=%s", av_err2str(result));
            result = 0;
            goto EXIT;
        }
        while (!result) {
            result = avcodec_receive_packet(stream->m_CodecCtx, packet);
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                result = 0;
                goto EXIT;
            } else if (result < 0) {
                LOGCATE("MediaRecorder::avcodec_receive_packet error. ret=%s", av_err2str(result));
                result = 0;
                goto EXIT;
            }
            //Write the compressed frame to the media file
            av_packet_rescale_ts(packet, stream->m_CodecCtx->time_base,
                                 stream->m_Stream->time_base);
            packet->stream_index = stream->m_Stream->index;
            result = av_interleaved_write_frame(m_AVFormatContext, packet);
            if (result < 0) {
                LOGCATE("MediaRecorder::av_interleaved_write_frame ret=%s", av_err2str(result));
                result = 0;
                goto EXIT;
            }
        }
    }
    EXIT:
    if (queueFrame) {
        PixImageUtils::pix_image_free(queueFrame->image);
        if (queueFrame) delete queueFrame;
    }
    return result;
}

int MediaRecord::codeAudioFrame(AVOutputStream *stream) {
    int result = 0;
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    while (m_AudioQueue.empty() && !m_Interrupt) {
        usleep(10 * 1000);
    }
    frame = stream->m_TmpFrame;
    AudioFrame *audioFrame = m_AudioQueue.front();
    m_AudioQueue.pop();
    if (audioFrame) {
        frame->data[0] = audioFrame->data;
        frame->nb_samples = audioFrame->dataSize / 4;
        frame->pts = stream->m_NextPts;
        stream->m_NextPts += frame->nb_samples;
    }
    if ((m_AudioQueue.empty() && m_Interrupt) || stream->m_EncodeEnd) frame = nullptr;
    if (frame) {
        int dst_nb_samples = av_rescale_rnd(
                swr_get_delay(stream->m_SwrCtx, stream->m_CodecCtx->sample_rate) +
                frame->nb_samples, stream->m_CodecCtx->sample_rate, stream->m_CodecCtx->sample_rate,
                AV_ROUND_UP);
        av_assert0(dst_nb_samples == frame->nb_samples);
        //when we pass a frame to the encoder, it may keep a reference to it internally make sure we do not overwrite it here
        result = av_frame_make_writable(stream->m_Frame);
        if (result < 0) {
            LOGCATE("MediaRecorder::av_frame_make_writable error");
            result = 1;
            goto EXIT;
        }
        // convert to destination format
        result = swr_convert(stream->m_SwrCtx,
                             stream->m_Frame->data, dst_nb_samples,
                             (const uint8_t **) frame->data, frame->nb_samples);
        if (result < 0) {
            LOGCATE("MediaRecorder::swr_convert error");
            result = 1;
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
    if (audioFrame) delete audioFrame;
    return result;
}

void MediaRecord::onRunAsy(MediaRecord *p) {
    AVOutputStream *vStream = &p->m_VideoStream;
    AVOutputStream *aStream = &p->m_AudioStream;
    while (!vStream->m_EncodeEnd || !aStream->m_EncodeEnd) {
//        double videoTimestamp = vStream->m_NextPts * av_q2d(vStream->m_CodecCtx->time_base);
//        double audioTimestamp = aStream->m_NextPts * av_q2d(aStream->m_CodecCtx->time_base);
//        if (!vStream->m_EncodeEnd && (aStream->m_EncodeEnd ||
//                                  av_compare_ts(vStream->m_NextPts, vStream->m_CodecCtx->time_base,
//                                                aStream->m_NextPts, aStream->m_CodecCtx->time_base) <=
//                                  0)) {
//            //视频和音频时间戳对齐，人对于声音比较敏感，防止出现视频声音播放结束画面还没结束的情况
//            if (audioTimestamp <= videoTimestamp && aStream->m_EncodeEnd) vStream->m_EncodeEnd = 1;
        vStream->m_EncodeEnd = p->codeVideoFrame(vStream);
//        } else {
//            aStream->m_EncodeEnd = p->codeAudioFrame(aStream);
//        }
    }
}

}





