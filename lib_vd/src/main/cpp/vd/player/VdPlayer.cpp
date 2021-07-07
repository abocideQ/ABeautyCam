#include "VdPlayer.h"

extern "C" {

void VdPlayer::onSource(char *source) {
    onStop();
    //video
    if (m_VideoRender == nullptr) {
        m_VideoRender = new VideoRender();
    }
    if (m_VideoCodec == nullptr) {
        m_VideoCodec = new VideoCodec();
    }
    m_VideoCodec->setRender(m_VideoRender);
    m_VideoCodec->onInit(source);
    //audio
    if (m_AudioRender == nullptr) {
        m_AudioRender = new AudioRender();
    }
    if (m_AudioCodec == nullptr) {
        m_AudioCodec = new AudioCodec();
    }
    m_AudioRender->onAudioCreate();
    m_AudioCodec->setRender(m_AudioRender);
    m_AudioCodec->onInit(source);
}

void VdPlayer::onSeekTo(int percent) {
    if (m_VideoCodec != nullptr && m_VideoRender != nullptr) {
        m_VideoCodec->onSeekTo(percent);
    }
    if (m_AudioCodec != nullptr && m_AudioRender != nullptr) {
        m_AudioCodec->onSeekTo(percent);
    }
}

void VdPlayer::onPlay() {
    if (m_VideoCodec != nullptr && m_VideoRender != nullptr) {
        m_VideoCodec->onResume();
        m_VideoRender->onResume();
    }
    if (m_AudioCodec != nullptr && m_AudioRender != nullptr) {
        m_AudioCodec->onResume();
        m_AudioRender->onResume();
    }
}

void VdPlayer::onPause() {
    if (m_VideoCodec != nullptr && m_VideoRender != nullptr) {
        m_VideoCodec->onPause();
        m_VideoRender->onPause();
    }
    if (m_AudioCodec != nullptr && m_AudioRender != nullptr) {
        m_AudioCodec->onPause();
        m_AudioRender->onPause();
    }
}

void VdPlayer::onStop() {
    if (m_VideoCodec != nullptr) {
        m_VideoCodec->onStop();
        m_VideoCodec->onRelease();
        m_VideoCodec = nullptr;
    }
    if (m_VideoRender != nullptr) {
        m_VideoRender->onStop();
    }
    if (m_AudioCodec != nullptr) {
        m_AudioCodec->onStop();
        m_AudioCodec->onRelease();
        m_AudioCodec = nullptr;
    }
    if (m_AudioRender != nullptr) {
        m_AudioRender->onStop();
        m_AudioRender->onRelease();
        m_AudioRender = nullptr;
    }
}

void VdPlayer::onRelease() {
    onStop();
    if (m_VideoRender != nullptr) {
        m_VideoRender->onRelease();
        m_VideoRender = nullptr;
    }
    m_Player = nullptr;
}

void VdPlayer::onSurfaceCreated() {
    if (m_VideoRender == nullptr) return;
    m_VideoRender->onSurfaceCreated();
}

void VdPlayer::onSurfaceChanged(int w, int h) {
    if (m_VideoRender == nullptr) return;
    m_VideoRender->onSurfaceChanged(w, h);
}

void VdPlayer::onDrawFrame() {
    if (m_VideoRender == nullptr) return;
    m_VideoRender->onDrawFrame();
}

VdPlayer *VdPlayer::m_Player = nullptr;
VdPlayer *VdPlayer::instance() {
    if (m_Player == nullptr) {
        m_Player = new VdPlayer();
    }
    return m_Player;
}
}







