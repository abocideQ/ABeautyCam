#include "BaseSL.h"

extern "C" {

int BaseSL::slProgramCreate() {
    int result = createEngine();
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL:: createEngine error =%d", result);
        return 0;
    }
    result = createMixer();
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::createMixer error =%d", result);
        return 0;
    }
    result = createPlayer();
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::createPlayer error =%d", result);
        return 0;
    }
    return result;
}

void BaseSL::slProgramDelete() {
    if (m_PlayerObj) {
        (*m_PlayerObj)->Destroy(m_PlayerObj);
        m_PlayerObj = nullptr;
        m_PlayerItf = nullptr;
    }
    if (m_MixerObj) {
        (*m_MixerObj)->Destroy(m_MixerObj);
        m_MixerObj = nullptr;
        m_MixerItf = nullptr;
    }
    if (m_EngineObj) {
        (*m_EngineObj)->Destroy(m_EngineObj);
        m_EngineObj = nullptr;
        m_EngineItf = nullptr;
    }
    m_BufferQueueItf = nullptr;
}

int BaseSL::createEngine() {
    SLresult result = slCreateEngine(&m_EngineObj, 0, nullptr, 0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::engine slCreateEngine error =%d", result);
        return 0;
    }
    //实例化
    result = (*m_EngineObj)->Realize(m_EngineObj, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::engine Realize error =%d", result);
        return 0;
    }
    result = (*m_EngineObj)->GetInterface(m_EngineObj, SL_IID_ENGINE, &m_EngineItf);
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::engine GetInterface error =%d", result);
        return 0;
    }
    return result;
}

int BaseSL::createMixer() {
    const SLInterfaceID faceId[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean faceReq[1] = {SL_BOOLEAN_FALSE};
    SLresult result = (*m_EngineItf)->CreateOutputMix(m_EngineItf, &m_MixerObj, 1, faceId, faceReq);
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::mixer CreateOutputMix error =%d", result);
        return 0;
    }
    result = (*m_MixerObj)->Realize(m_MixerObj, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::mixer Realize error =%d", result);
        return 0;
    }
    result = (*m_MixerObj)->GetInterface(m_MixerObj, SL_IID_OUTPUTMIX, &m_MixerItf);
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::mixer GetInterface error =%d", result);
        return 0;
    }
    return result;
}

int BaseSL::createPlayer() {
    SLDataLocator_AndroidBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                      2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//format pcm
            (SLuint32) 2,//channel 2
            SL_SAMPLINGRATE_44_1,//44100hz
            SL_PCMSAMPLEFORMAT_FIXED_16,//bit of sample 16
            SL_PCMSAMPLEFORMAT_FIXED_16,//container size 16
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//channel mask left right
            SL_BYTEORDER_LITTLEENDIAN //endianness
    };
    SLDataSource slDataSource = {&android_queue, &pcm};

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, m_MixerObj};
    SLDataSink slDataSink = {&outputMix, nullptr};

    const SLInterfaceID faceId[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean faceReq[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    SLresult result = (*m_EngineItf)->CreateAudioPlayer(m_EngineItf, &m_PlayerObj, &slDataSource,
                                                        &slDataSink, 3, faceId, faceReq);
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::vd.player CreateAudioPlayer error =%d", result);
        return 0;
    }
    result = (*m_PlayerObj)->Realize(m_PlayerObj, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::vd.player Realize error =%d", result);
        return 0;
    }
    result = (*m_PlayerObj)->GetInterface(m_PlayerObj, SL_IID_PLAY, &m_PlayerItf);
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::vd.player GetInterface error =%d", result);
        return 0;
    }
    result = (*m_PlayerObj)->GetInterface(m_PlayerObj, SL_IID_BUFFERQUEUE, &m_BufferQueueItf);
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::vd.player buffer GetInterface error =%d", result);
        return 0;
    }
    result = (*m_PlayerObj)->GetInterface(m_PlayerObj, SL_IID_VOLUME, &m_VolumeItf);
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::vd.player volume GetInterface error =%d", result);
        return 0;
    }
    result = (*m_BufferQueueItf)->RegisterCallback(m_BufferQueueItf, AudioPlayerCallback, this);
    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("BaseSL::vd.player RegisterCallback error =%d", result);
        return 0;
    }
    return result;
}

void BaseSL::handlerCallBack(SLAndroidSimpleBufferQueueItf bufferQueue) {
}

void BaseSL::AudioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context) {
    BaseSL *instance = static_cast<BaseSL *>(context);
    instance->handlerCallBack(bufferQueue);
}
}

