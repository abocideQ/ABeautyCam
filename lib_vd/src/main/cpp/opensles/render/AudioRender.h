#ifndef FFMPEGTEST_AUDIORENDER_H
#define FFMPEGTEST_AUDIORENDER_H

#include "BaseSL.h"
#include "PixAudio.h"

#include <thread>
#include <mutex>

class AudioRender : public BaseSL {
public:
    void onBuffer(uint8_t *data, int size);

    void onAudioCreate();

    void onDrawFrame();

    void onResume();

    void onPause();

    void onStop();

    void onRelease();

protected:
    //data
    AudioFrame *m_Frame = nullptr;
    //线程
    std::thread *m_Thread = nullptr;
    volatile int m_Interrupt = 0;
    //互斥锁
    static std::mutex m_Mutex;
private:
    static void audioRunAsy(AudioRender *ptr);
};


#endif //FFMPEGTEST_AUDIORENDER_H
