#ifndef VDMAKE_VIDEORENDER_H
#define VDMAKE_VIDEORENDER_H

#include <thread>
#include <mutex>
#include <unistd.h>

#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"

#include "Log.h"
#include "PixImage.h"
#include "GLUtils.h"

#include "FaceCvDetection.h"
#include "FaceCvTrack.h"
#include "FaceCnnDetection.h"

typedef struct _tag_render {
    PixImage *pixel;
    std::vector<cv::Rect> faces;
    std::vector<cv::Rect> eyes;
    std::vector<cv::Rect> noses;
    std::vector<cv::Rect> mouths;

    _tag_render() {
        pixel = nullptr;
    }
} VRender;

typedef struct _tag_fbo {
    const char *shader;
    GLuint m_FboMix[1];
    GLuint m_FboMix_Program[1];
    GLuint m_FboMix_VAO[1];
    GLuint m_FboMix_Texture[1];
} FrameBufferObj;

typedef void (*OnRenderFrameCallback)(void *, int, int, int, uint8_t *); // 回调函数 -> Record

class VideoRender {

public:

    //face
    void onFace(char *s1, char *s2, char *s3, char *s4, char *s5, int faceI);

    //normal
    void onBuffer(int format, int width, int height, int lineSize[3], uint8_t *data);

    void onBuffer(PixImage *pix);

    uint8_t *onFrameBuffer();

    int onFrameBufferSize();

    void onCamera(bool camera);

    void onRotate(float viewRot, float modelRot);

    void onSurfaceCreated();

    void onFrameMixCreated();

    void onSurfaceChanged(int w, int h);

    void onMatrix(const char *gl_name, float viewRot, float modelRot) const;

    void onDrawFrame();

    GLuint onDrawFrameMix(int width, int height,
                          std::vector<cv::Rect> faces,
                          std::vector<cv::Rect> eyes,
                          std::vector<cv::Rect> noses,
                          std::vector<cv::Rect> mouths);

    void onResume();

    void onPause();

    void onStop();

    void onRelease();

    void onReleaseRenders();

    void onReleaseRender(VRender *render);

    void onFrameBufferUpdate(int width, int height);

    OnRenderFrameCallback m_RenderFrameCallback = nullptr;
    void *m_CallbackContext = nullptr;

    void SetRenderCallback(void *ctx, OnRenderFrameCallback callback) {
        m_CallbackContext = ctx;
        m_RenderFrameCallback = callback;
    }

protected:
    //数据
    std::queue<VRender *> m_VRenderQueue;
    int m_Width_display = 0;
    int m_Height_display = 0;
    float m_ViewRot = 0.0f;
    float m_ModelRot = 0.0f;
    bool m_CameraData = false;
    //framebuffer data 录制用数据
    uint8_t *m_FrameBuffer = nullptr;
    int m_FrameBufferSize = 0;

    //normal 展示用
    GLuint m_Program;
    GLuint m_VBO[4];
    GLuint m_VAO[1];
    GLuint m_Texture[3];
    //fbo 展示用
    GLuint m_Fbo[1];
    GLuint m_Fbo_Program[3];//display:yuv420 nv21/nv12 rgb
    GLuint m_Fbo_VAO[1];
    GLuint m_Fbo_Texture[1];
    //fbo 处理用
    std::vector<FrameBufferObj> m_FboMixes;
    //互斥锁
    static std::mutex m_Mutex;
    //===
    volatile bool m_Interrupt = false;
private:
    //cv or faceCnn
    int FACE_ON = 0; //1 cv 2 cvTrack 3 faceCnn
    //opencv
    FaceCvDetection *m_FaceCvDetection = nullptr;
    //cvTrack
    FaceCvTrack *m_FaceTrack = nullptr;
    //faceCnn
    FaceCnnDetection *mFaceCnnDetection = nullptr;
};

#endif //VDMAKE_VIDEORENDER_H
