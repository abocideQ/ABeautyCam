#ifndef FFMPEGTEST_BASESL_H
#define FFMPEGTEST_BASESL_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "Log.h"

class BaseSL {
public:
    //SL引擎
    SLObjectItf m_EngineObj;
    SLEngineItf m_EngineItf;
    //SL输出
    SLObjectItf m_MixerObj;
    SLOutputMixItf m_MixerItf;
    //SLPlayer
    SLObjectItf m_PlayerObj;
    SLPlayItf m_PlayerItf;
    //SL音量
    SLVolumeItf m_VolumeItf;
    //SLBuffer
    SLAndroidSimpleBufferQueueItf m_BufferQueueItf;

    int slProgramCreate();

    void slProgramDelete();

protected:

    int createEngine();

    int createMixer();

    int createPlayer();

private:
    static void AudioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context);

    void handlerCallBack(SLAndroidSimpleBufferQueueItf bufferQueue);
};

#endif //FFMPEGTEST_BASESL_H
