#include "VdCameraRender.h"

extern "C" {

void VdCameraRender::onFace(char *s1, char *s2, char *s3, char *s4, char *s5, int faceI) {
    m_Render->onFace(s1, s2, s3, s4, s5, faceI);
}

void VdCameraRender::onBuffer(int format, int w, int h, uint8_t *data) {
    m_Render->onBuffer(format, w, h, nullptr, data);
}

void VdCameraRender::onRotate(float rot, bool modelRot) {
    if (modelRot) {
        m_Render->onRotate(rot, 0.0f);
    } else {
        m_Render->onRotate(rot, 180.0f);
    }
}

void VdCameraRender::onSurfaceCreated() {
    m_Render->onCamera(true);
    m_Render->onSurfaceCreated();
}

void VdCameraRender::onSurfaceChanged(int w, int h) {
    m_Render->onSurfaceChanged(w, h);
}

void VdCameraRender::onDrawFrame() {
    m_Render->onDrawFrame();
}

void VdCameraRender::onRelease() {
    m_Render->onRelease();
    m_Sample = nullptr;
    m_Sample = nullptr;
}

uint8_t *VdCameraRender::onBuffer() {
    return m_Render->onFrameBuffer();
}

int VdCameraRender::onBufferSize() {
    return m_Render->onFrameBufferSize();
}

VdCameraRender *VdCameraRender::m_Sample;
VideoRender *VdCameraRender::m_Render;

VdCameraRender *VdCameraRender::instance() {
    if (m_Sample == nullptr) {
        m_Sample = new VdCameraRender();
        m_Render = new VideoRender();
    }
    return m_Sample;
}

VideoRender *VdCameraRender::render() {
    return m_Render;
}
}


