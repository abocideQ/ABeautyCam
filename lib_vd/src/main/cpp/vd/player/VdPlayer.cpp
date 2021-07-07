#include "VdPlayer.h"

extern "C" {

void VdPlayer::onSource(char *source) {
    onStop();
    //video
    if (m_VideoRender == nullptr) {
        m_VideoRender = new VideoRender();
    }
    if (m_VideoDecode == nullptr) {
        m_VideoDecode = new VideoDecode();
    }
    m_VideoDecode->setRender(m_VideoRender);
    m_VideoDecode->onInit(source);
    //audio
    if (m_AudioRender == nullptr) {
        m_AudioRender = new AudioRender();
    }
    if (m_AudioDecode == nullptr) {
        m_AudioDecode = new AudioDecode();
    }
    m_AudioRender->onAudioCreate();
    m_AudioDecode->setRender(m_AudioRender);
    m_AudioDecode->onInit(source);
}

void VdPlayer::onSeekTo(int percent) {
    if (m_VideoDecode != nullptr && m_VideoRender != nullptr) {
        m_VideoDecode->onSeekTo(percent);
    }
    if (m_AudioDecode != nullptr && m_AudioRender != nullptr) {
        m_AudioDecode->onSeekTo(percent);
    }
}

void VdPlayer::onPlay() {
    if (m_VideoDecode != nullptr && m_VideoRender != nullptr) {
        m_VideoDecode->onResume();
        m_VideoRender->onResume();
    }
    if (m_AudioDecode != nullptr && m_AudioRender != nullptr) {
        m_AudioDecode->onResume();
        m_AudioRender->onResume();
    }
}

void VdPlayer::onPause() {
    if (m_VideoDecode != nullptr && m_VideoRender != nullptr) {
        m_VideoDecode->onPause();
        m_VideoRender->onPause();
    }
    if (m_AudioDecode != nullptr && m_AudioRender != nullptr) {
        m_AudioDecode->onPause();
        m_AudioRender->onPause();
    }
}

void VdPlayer::onStop() {
    if (m_VideoDecode != nullptr) {
        m_VideoDecode->onStop();
        m_VideoDecode->onRelease();
        m_VideoDecode = nullptr;
    }
    if (m_VideoRender != nullptr) {
        m_VideoRender->onStop();
    }
    if (m_AudioDecode != nullptr) {
        m_AudioDecode->onStop();
        m_AudioDecode->onRelease();
        m_AudioDecode = nullptr;
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







