#ifndef VDMAKE_AUDIOCODEC_H
#define VDMAKE_AUDIOCODEC_H

#include "BaseDecode.h"
#include "AudioRender.h"

class AudioDecode : public BaseDecode {
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


#endif //VDMAKE_AUDIOCODEC_H
