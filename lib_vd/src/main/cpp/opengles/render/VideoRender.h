#ifndef VDMAKE_VIDEORENDER_H
#define VDMAKE_VIDEORENDER_H

#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"
#include "FaceCheck.h"
#include "PixImage.h"
#include "GLUtils.h"
#include "Log.h"

#include <thread>
#include <mutex>

typedef void (*OnRenderFrameCallback)(void *, int, int, int, uint8_t *); // 回调函数 -> Record

class VideoRender {

public:

    //face
    void onFaceInit(char *face, char *eye, char *nose, char *mouth);

    void onFaceBuffer(int format, int w, int h, uint8_t *data);

    //normal
    void onBuffer(PixImage *image);

    uint8_t *onBuffer();

    int onBufferSize();

    void onCamera(bool camera);

    void onRotate(float viewRot, int modelRot);

    void onSurfaceCreated();

    void onSurfaceChanged(int w, int h);

    void onMatrix(const char *gl_name, float viewRot, float modelRot);

    void onDrawFrame();

    void onResume();

    void onPause();

    void onStop();

    void onRelease();

    void onFrameBufferUpdate();

    OnRenderFrameCallback m_RenderFrameCallback = nullptr;
    void *m_CallbackContext = nullptr;

    void SetRenderCallback(void *ctx, OnRenderFrameCallback callback) {
        m_CallbackContext = ctx;
        m_RenderFrameCallback = callback;
    }

protected:
    //数据
    PixImage *m_Image = nullptr;
    int m_Width_display = 0;
    int m_Height_display = 0;
    uint8_t *m_data = nullptr;
    int m_dataSize = 0;
    float m_ViewRot = 0.0f;
    float m_ModelRot = 0.0f;
    bool m_CameraData = false;

    //VBO
    GLuint m_VBO[4];
    //显示部分
    GLuint m_Program;
    GLuint m_Texture[3];
    GLuint m_VAO[1];
    //离屏部分
    GLuint m_Program_Fbo_YUV420P;
    GLuint m_Program_Fbo_NV21;
    GLuint m_Program_Fbo_RGB;
    GLuint m_Texture_Fbo[1];
    GLuint m_VAO_Fbo[1];
    GLuint m_Fbo[1];
    //互斥锁
    static std::mutex m_Mutex;
    //===
    volatile bool m_Interrupt = false;

    //opencv
    FaceCheck *m_Face = nullptr;
    char *m_face_model = nullptr;
    char *m_eye_model = nullptr;
    char *m_nose_model = nullptr;
    char *m_mouth_model = nullptr;
};


#endif //VDMAKE_VIDEORENDER_H
