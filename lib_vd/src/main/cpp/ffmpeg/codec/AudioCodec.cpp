#include "AudioCodec.h"

extern "C" {

void AudioCodec::setRender(AudioRender *render) {
    m_Render = render;
}

void AudioCodec::onInit(char *url) {
    BaseCodec::onInit(url, AVMEDIA_TYPE_AUDIO);
}

void AudioCodec::onResume() {
    BaseCodec::onResume();
}

void AudioCodec::onPause() {
    BaseCodec::onPause();
}

void AudioCodec::onStop() {
    BaseCodec::onStop();
}

void AudioCodec::onRelease() {
    BaseCodec::onRelease();
    if (m_AudioOutBuffer) {
        free(m_AudioOutBuffer);
        m_AudioOutBuffer = nullptr;
    }
    if (m_SwrContext) {
        swr_free(&m_SwrContext);
        m_SwrContext = nullptr;
    }
}

void AudioCodec::codecHandler(AVFrame *frame) {
    if (m_SwrContext == nullptr) {
        //音频采样工具Context初始化
        m_SwrContext = swr_alloc();
        // 音频编码声道格式
        uint64_t AUDIO_DST_CHANNEL_LAYOUT = AV_CH_LAYOUT_STEREO;
        av_opt_set_int(m_SwrContext, "in_channel_layout", m_AVCodecContext->channel_layout, 0);
        av_opt_set_int(m_SwrContext, "out_channel_layout", AUDIO_DST_CHANNEL_LAYOUT, 0);
        // 音频编码采样率
        int AUDIO_DST_SAMPLE_RATE = 44100;
        av_opt_set_int(m_SwrContext, "in_sample_rate", m_AVCodecContext->sample_rate, 0);
        av_opt_set_int(m_SwrContext, "out_sample_rate", AUDIO_DST_SAMPLE_RATE, 0);
        // 音频采样格式
        AVSampleFormat AUDIO_DST_SAMPLE_FMT = AV_SAMPLE_FMT_S16;
        av_opt_set_sample_fmt(m_SwrContext, "in_sample_fmt", m_AVCodecContext->sample_fmt, 0);
        av_opt_set_sample_fmt(m_SwrContext, "out_sample_fmt", AUDIO_DST_SAMPLE_FMT, 0);
        swr_init(m_SwrContext);
        // 音频编码通道数
        int AUDIO_DST_CHANNEL_COUNTS = 2;
        // 音频编码比特率
        int AUDIO_DST_BIT_RATE = 64000;
        // ACC音频一帧采样数
        int ACC_NB_SAMPLES = 1024;
        int samples = (int) av_rescale_rnd(ACC_NB_SAMPLES, AUDIO_DST_SAMPLE_RATE,
                                           m_AVCodecContext->sample_rate, AV_ROUND_UP);
        m_BufferSize = av_samples_get_buffer_size(NULL, AUDIO_DST_CHANNEL_COUNTS, samples,
                                                  AUDIO_DST_SAMPLE_FMT, 1);
        m_AudioOutBuffer = (uint8_t *) malloc(m_BufferSize);
    }
    int result = swr_convert(m_SwrContext, &m_AudioOutBuffer, m_BufferSize / 2,
                             (const uint8_t **) frame->data, frame->nb_samples);
    if (result > 0) {
        m_Render->onBuffer(m_AudioOutBuffer, m_BufferSize);
    }
}
}
