#include "VideoCodec.h"

extern "C" {

void VideoCodec::setRender(VideoRender *render) {
    m_Render = render;
}

void VideoCodec::onInit(char *url) {
    BaseCodec::onInit(url, AVMEDIA_TYPE_VIDEO);
}

void VideoCodec::onResume() {
    BaseCodec::onResume();
}

void VideoCodec::onPause() {
    BaseCodec::onPause();
}

void VideoCodec::onStop() {
    BaseCodec::onStop();
}

void VideoCodec::onRelease() {
    BaseCodec::onRelease();
    if (m_SwsContext != nullptr) {
        sws_freeContext(m_SwsContext);
        m_SwsContext = nullptr;
    }
    if (m_FrameScale != nullptr) {
        av_frame_free(&m_FrameScale);
        m_FrameScale = nullptr;
    }
    if (m_FrameScaleBuffer != nullptr) {
        free(m_FrameScaleBuffer);
        m_FrameScaleBuffer = nullptr;
    }
}

void VideoCodec::codecHandler(AVFrame *frame) {
    PixImage *image = nullptr;
    int width = frame->width;
    int height = frame->height;
    if (m_AVCodecContext->pix_fmt == AV_PIX_FMT_YUV420P ||
        m_AVCodecContext->pix_fmt == AV_PIX_FMT_YUVJ420P) {
        image = PixImageUtils::pix_image_get(IMAGE_FORMAT_YUV420P, width, height, frame->linesize,
                                             frame->data);
        if (frame->data[0] && frame->data[1] && !frame->data[2] &&
            frame->linesize[0] == frame->linesize[1] && frame->linesize[2] == 0) {
            //h264 mediacodec decoder is NV12 兼容某些设备可能出现的格式不匹配问题
            image->format = IMAGE_FORMAT_NV12;
            LOGCATE("yuv420 wrong , try nv12");
        }
    } else if (m_AVCodecContext->pix_fmt == AV_PIX_FMT_NV21) {
        image = PixImageUtils::pix_image_get(IMAGE_FORMAT_NV21, width, height, frame->linesize,
                                             frame->data);
    } else if (m_AVCodecContext->pix_fmt == AV_PIX_FMT_NV12) {
        image = PixImageUtils::pix_image_get(IMAGE_FORMAT_NV12, width, height, frame->linesize,
                                             frame->data);
    } else if (m_AVCodecContext->pix_fmt == AV_PIX_FMT_RGBA) {
        image = PixImageUtils::pix_image_get(IMAGE_FORMAT_RGBA, width, height, frame->linesize,
                                             frame->data);
    } else {
        if (m_SwsContext == nullptr) {
            int width = m_AVCodecContext->width;
            int height = m_AVCodecContext->height;
            int align = 1;//该对齐基数align必须是2的n次方,并width/align为整数.(1280 -> 1288 align为1影响性能)
            m_FrameScale = av_frame_alloc();
            m_BufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, width, height, align);
            m_FrameScaleBuffer = (uint8_t *) av_malloc(m_BufferSize * sizeof(uint8_t));
            av_image_fill_arrays(m_FrameScale->data, m_FrameScale->linesize, m_FrameScaleBuffer,
                                 AV_PIX_FMT_RGBA, width, height, align);
            m_SwsContext = sws_getContext(width, height, m_AVCodecContext->pix_fmt,
                                          width, height, AV_PIX_FMT_RGBA,
                                          SWS_FAST_BILINEAR, NULL, NULL, NULL);
        }
        sws_scale(m_SwsContext, frame->data, frame->linesize, 0, frame->height,
                  m_FrameScale->data, m_FrameScale->linesize);
        image = PixImageUtils::pix_image_get(IMAGE_FORMAT_RGBA, width, height,
                                             m_FrameScale->linesize,
                                             m_FrameScale->data);
    }
    if (m_Render != nullptr) {
        m_Render->onBuffer(image);
    }
}
}
