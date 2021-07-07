#ifndef FFMPEGTEST_VIDEORENDER_H
#define FFMPEGTEST_VIDEORENDER_H

#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"
#include "GLUtils.h"
#include "PixImage.h"
#include "Log.h"

#include <thread>
#include <mutex>

class VideoRender {

public:

    void onBuffer(PixImage *image);

    uint8_t *onBuffer();

    int onBufferSize();

    void onRotate(float rotate);

    void onSurfaceCreated();

    void onSurfaceChanged(int w, int h);

    void onDrawFrame();

    void onResume();

    void onPause();

    void onStop();

    void onRelease();

protected:
    //数据
    PixImage *m_Image = nullptr;
    int m_Width_display = 0;
    int m_Height_display = 0;
    uint8_t *m_data = nullptr;
    int m_dataSize = 0;
    float m_Rotation = 0.0f;

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
};


#endif //FFMPEGTEST_VIDEORENDER_H