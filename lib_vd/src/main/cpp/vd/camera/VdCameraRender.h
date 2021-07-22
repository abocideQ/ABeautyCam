#ifndef VDMAKE_VDCAMERARENDER_H
#define VDMAKE_VDCAMERARENDER_H

#include "VideoRender.h"

class VdCameraRender {
public:
    //face opencv
    void onFaceCV(char *face, char *eye, char *nose, char *mouth);

    //normal
    void onBuffer(int format, int w, int h, uint8_t *data);

    void onBufferFacePlus(int format, int w, int h, uint8_t *data,
                          int faceX, int faceY,
                          int faceW, int faceH,
                          int eyeLX, int eyeLY,
                          int eyeRX, int eyeRY);

    uint8_t *onBuffer();

    int onBufferSize();

    void onRotate(float rot, bool modelRot);

    void onSurfaceCreated();

    void onSurfaceChanged(int w, int h);

    void onDrawFrame();

    void onRelease();

    static VdCameraRender *instance();

    static VideoRender *render();

private:
    static VdCameraRender *m_Sample;
    static VideoRender *m_Render;
};


#endif //VDMAKE_VDCAMERARENDER_H
