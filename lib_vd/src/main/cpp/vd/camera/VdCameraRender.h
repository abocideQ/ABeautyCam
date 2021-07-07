#ifndef VDMAKE_VDCAMERARENDER_H
#define VDMAKE_VDCAMERARENDER_H

#include "VideoRender.h"

class VdCameraRender {
public:
    void onBuffer(int format, int w, int h, uint8_t *data);

    uint8_t *onBuffer();

    int onBufferSize();

    void onRotate(float rotate);

    void onSurfaceCreated();

    void onSurfaceChanged(int w, int h);

    void onDrawFrame();

    void onRelease();

    static VdCameraRender *instance();

private:
    static VdCameraRender *m_Sample;
    static VideoRender *m_Render;
};


#endif //VDMAKE_VDCAMERARENDER_H