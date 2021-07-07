#ifndef FFMPEGTEST_AUDIOCODEC_H
#define FFMPEGTEST_AUDIOCODEC_H

#include "BaseCodec.h"
#include "AudioRender.h"

class AudioCodec : public BaseCodec {
public:
    void setRender(AudioRender *render);

    void onInit(char *url);

    void onResume();

    void onPause();

    void onStop();

    void onRelease();

protected:
    //音频采样工具Context
    SwrContext *m_SwrContext = nullptr;
    //音频数据
    int m_BufferSize = 0;
    uint8_t *m_AudioOutBuffer = nullptr;

    void codecHandler(AVFrame *frame);

private:
    AudioRender *m_Render = nullptr;
};


#endif //FFMPEGTEST_AUDIOCODEC_H
