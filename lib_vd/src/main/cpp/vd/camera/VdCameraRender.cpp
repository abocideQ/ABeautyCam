#include "VdCameraRender.h"

extern "C" {
void VdCameraRender::onBuffer(int format, int w, int h, uint8_t *data) {
    PixImage *image = nullptr;
    if (format == 1) {
        image = PixImageUtils::pix_image_get(IMAGE_FORMAT_YUV420P, w, h, data);
    } else if (format == 2) {
        image = PixImageUtils::pix_image_get(IMAGE_FORMAT_NV21, w, h, data);
    } else {
        image = PixImageUtils::pix_image_get(IMAGE_FORMAT_RGBA, w, h, data);
    }
    m_Render->onBuffer(image);
}

void VdCameraRender::onRotate(float rotate) {
    m_Render->onRotate(rotate);
}

void VdCameraRender::onSurfaceCreated() {
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
    return m_Render->onBuffer();
}

int VdCameraRender::onBufferSize() {
    return m_Render->onBufferSize();
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


