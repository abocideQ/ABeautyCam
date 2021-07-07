#include "AudioRender.h"

extern "C" {
std::mutex AudioRender::m_Mutex;
void AudioRender::onBuffer(uint8_t *data, int size) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_Frame != nullptr && m_Frame->data != nullptr) {
        free(m_Frame->data);
        m_Frame = nullptr;
    }
    m_Frame = new AudioFrame(data, size);
}

void AudioRender::onAudioCreate() {
    m_Thread = new std::thread(audioRunAsy, this);
}

void AudioRender::audioRunAsy(AudioRender *ptr) {
    ptr->onDrawFrame();
}

void AudioRender::onDrawFrame() {
    slProgramCreate();
    if (m_PlayerItf == nullptr) return;
    (*m_PlayerItf)->SetPlayState(m_PlayerItf, SL_PLAYSTATE_PLAYING);
    for (;;) {
        while (m_Interrupt == -1) {
            continue;
        }
        if (m_Interrupt == 1) {
            break;
        }
        std::unique_lock<std::mutex> lock(m_Mutex);
        if (m_PlayerItf && m_Frame != nullptr) {
            SLresult result = (*m_BufferQueueItf)->Enqueue(m_BufferQueueItf, m_Frame->data,
                                                           (SLuint32) m_Frame->dataSize);
            if (result == SL_RESULT_SUCCESS) {
                if (m_Frame != nullptr && m_Frame->data != nullptr) {
                    free(m_Frame->data);
                    m_Frame = nullptr;
                }
            }
        }
        lock.unlock();
    }
}

void AudioRender::onResume() {
    m_Interrupt = 0;
    if (m_PlayerItf) {
        (*m_PlayerItf)->SetPlayState(m_PlayerItf, SL_PLAYSTATE_PLAYING);
    }
}

void AudioRender::onPause() {
    m_Interrupt = -1;
    if (m_PlayerItf) {
        (*m_PlayerItf)->SetPlayState(m_PlayerItf, SL_PLAYSTATE_PAUSED);
    }
}

void AudioRender::onStop() {
    m_Interrupt = 1;
    if (m_PlayerItf) {
        (*m_PlayerItf)->SetPlayState(m_PlayerItf, SL_PLAYSTATE_STOPPED);
    }
    if (m_Thread) {
        m_Thread->join();
        delete m_Thread;
        m_Thread = nullptr;
    }
}

void AudioRender::onRelease() {
    m_Interrupt = 1;
    if (m_PlayerItf) {
        (*m_PlayerItf)->SetPlayState(m_PlayerItf, SL_PLAYSTATE_STOPPED);
    }
    slProgramDelete();
}
}


