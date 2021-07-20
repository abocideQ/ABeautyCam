#include "VideoRender.h"
#include "Shaders.h"

extern "C" {
std::mutex VideoRender::m_Mutex;
const float Location_Vertex[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
};
const float Location_Texture[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
};
const float Location_Texture_Fbo[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
};
const int Location_Indices[] = {
        0, 1, 2, 1, 3, 2
};

void VideoRender::onFace(char *face, char *eye, char *nose, char *mouth) {
    if (face == nullptr || eye == nullptr || nose == nullptr || mouth == nullptr) {
        m_Interrupt_cv = 1;
        m_Thread_cv->join();
        delete m_Thread_cv;
        m_Thread_cv = nullptr;
        return;
    }
    std::unique_lock<std::mutex> lock(m_Mutex_cv);//加锁
    m_Interrupt_cv = 0;
    m_Face = new FaceCheck();
    m_Face->onModelSource(face, eye, nose, mouth);
    m_Thread_cv = new std::thread(onFaceLoop, this);
    lock.unlock();
}

void VideoRender::onFaceLoop(VideoRender *p) {
    while (true) {
        if (p == nullptr) return;
        if (p->m_Interrupt_cv == 1 || p->m_Interrupt) return;
        if (p->m_Image == nullptr || p->m_Image->origin == nullptr) {
            usleep(10 * 50000);
            continue;
        }
        int w = p->m_Image->width;
        int h = p->m_Image->height;
        uint8_t *data = p->m_Image->origin;
        std::unique_lock<std::mutex> lock(p->m_Mutex_cv);//加锁
        p->faces.clear();
        p->eyes.clear();
        p->noses.clear();
        p->mouths.clear();
        p->m_Face->onFaces(w, h, data, p->faces, p->eyes, p->noses, p->mouths);
        usleep(10 * 50000);
        lock.unlock();
    }
}

void VideoRender::onBuffer(int format, int w, int h, int lineSize[3], uint8_t *data) {
    if (data == nullptr || format == 0 || w == 0 | h == 0) return;
    std::unique_lock<std::mutex> lock(m_Mutex);//加锁
    PixImageUtils::pix_image_free(m_Image);
    if (format == 1) {
        format = IMAGE_FORMAT_YUV420P;
    } else if (format == 2 || format == 3) {
        format = IMAGE_FORMAT_NV21;
    } else {
        format = IMAGE_FORMAT_RGBA;
    }
    if (lineSize == nullptr) {
        m_Image = PixImageUtils::pix_image_get(format, w, h, data);
    } else {
        m_Image = PixImageUtils::pix_image_get(format, w, h, lineSize, &data);
    }
}

void VideoRender::onBuffer(PixImage *pix) {
    if (pix == nullptr || pix->format == 0 || pix->width == 0 | pix->height == 0) return;
    std::unique_lock<std::mutex> lock(m_Mutex);//加锁
    PixImageUtils::pix_image_free(m_Image);
    m_Image = pix;
}

uint8_t *VideoRender::onFrameBuffer() {
    return m_data;
}

int VideoRender::onFrameBufferSize() {
    return m_dataSize;
}

void VideoRender::onCamera(bool camera) {
    m_CameraData = camera;
}

void VideoRender::onRotate(float viewRot, int modelRot) {
    m_ViewRot = viewRot;
    m_ModelRot = modelRot;
    m_Image = nullptr;
}

void VideoRender::onSurfaceCreated() {
    m_Program = GLUtils::glProgram(ShaderVertex, ShaderFragment);
    m_Program_Fbo_YUV420P = GLUtils::glProgram(ShaderVertex_FBO, ShaderFragment_FBO_YUV420p);
    m_Program_Fbo_NV21 = GLUtils::glProgram(ShaderVertex_FBO, ShaderFragment_FBO_NV21);
    m_Program_Fbo_RGB = GLUtils::glProgram(ShaderVertex_FBO, ShaderFragment_FBO_RGB);
    if (m_Program == GL_NONE) return;
    if (m_Program_Fbo_YUV420P == GL_NONE) return;
    if (m_Program_Fbo_NV21 == GL_NONE) return;
    if (m_Program_Fbo_RGB == GL_NONE) return;
    //vbo
    glGenBuffers(4, m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Location_Vertex), Location_Vertex,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Location_Texture), Location_Texture,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Location_Texture_Fbo), Location_Texture_Fbo,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBO[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Location_Indices), Location_Indices,
                 GL_STATIC_DRAW);
    GL_ERROR_CHECK();
    //offscreen
    glGenVertexArrays(1, m_VAO_Fbo);
    glBindVertexArray(m_VAO_Fbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[2]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBO[3]);

    glGenTextures(1, m_Texture_Fbo);
    glBindTexture(GL_TEXTURE_2D, m_Texture_Fbo[0]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenFramebuffers(1, m_Fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo[0]);
    glBindTexture(GL_TEXTURE_2D, m_Texture_Fbo[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture_Fbo[0],
                           0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGCATE("gl_error::CreateFrameBufferObj status != GL_FRAMEBUFFER_COMPLETE");
    }
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    GL_ERROR_CHECK();
    //normal
    glGenVertexArrays(1, m_VAO);
    glBindVertexArray(m_VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBO[3]);

    glGenTextures(3, m_Texture);
    glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, m_Texture[1]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, m_Texture[2]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL_ERROR_CHECK();
}

void VideoRender::onSurfaceChanged(int width, int height) {
    m_Width_display = width;
    m_Height_display = height;
}

void VideoRender::onMatrix(const char *gl_name, float viewRot, float modelRot) {
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                       glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::rotate(view, glm::radians(viewRot), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(modelRot), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 mat4Matrix = glm::mat4(1.0f);
    mat4Matrix = projection * view * model;
    unsigned int vMatrix = glGetUniformLocation(m_Program, gl_name);
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, glm::value_ptr(mat4Matrix));
}

void VideoRender::onDrawFrame() {
    if (m_Program == GL_NONE) return;
    if (m_Program_Fbo_YUV420P == GL_NONE) return;
    if (m_Program_Fbo_NV21 == GL_NONE) return;
    if (m_Program_Fbo_RGB == GL_NONE) return;
    if (m_Image == nullptr)return;
    if (m_Image->width == 0) return;
    if (m_Image->height == 0) return;
    if (m_Image->format == 0) return;
    //textureImage2d
    std::unique_lock<std::mutex> lock(m_Mutex);
    if (m_Image->format == IMAGE_FORMAT_YUV420P) {
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_Image->width, m_Image->height, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, m_Image->plane[0]);
        glBindTexture(GL_TEXTURE_2D, m_Texture[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_Image->width / 2, m_Image->height / 2, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, m_Image->plane[1]);
        glBindTexture(GL_TEXTURE_2D, m_Texture[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_Image->width / 2, m_Image->height / 2, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, m_Image->plane[2]);
    } else if (m_Image->format == IMAGE_FORMAT_NV21 || m_Image->format == IMAGE_FORMAT_NV12) {
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_Image->width, m_Image->height, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, m_Image->plane[0]);
        glBindTexture(GL_TEXTURE_2D, m_Texture[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, m_Image->width / 2, m_Image->height / 2,
                     0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, m_Image->plane[1]);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Image->width, m_Image->height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, m_Image->plane[0]);
    }
    lock.unlock();
    //offscreen
    if (m_CameraData) {
        glBindTexture(GL_TEXTURE_2D, m_Texture_Fbo[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Image->height, m_Image->width, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glViewport(0, 0, m_Image->height, m_Image->width);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_Texture_Fbo[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Image->width, m_Image->height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glViewport(0, 0, m_Image->width, m_Image->height);
    }
    if (m_Image->format == IMAGE_FORMAT_YUV420P) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo[0]);
        glUseProgram(m_Program_Fbo_YUV420P);
        glBindVertexArray(m_VAO_Fbo[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_Texture[1]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_Texture[2]);
        GLint textureY = glGetUniformLocation(m_Program_Fbo_YUV420P, "s_textureY");
        GLint textureU = glGetUniformLocation(m_Program_Fbo_YUV420P, "s_textureU");
        GLint textureV = glGetUniformLocation(m_Program_Fbo_YUV420P, "s_textureV");
        glUniform1i(textureY, 0);
        glUniform1i(textureU, 1);
        glUniform1i(textureV, 2);
    } else if (m_Image->format == IMAGE_FORMAT_NV21 || m_Image->format == IMAGE_FORMAT_NV12) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo[0]);
        glUseProgram(m_Program_Fbo_NV21);
        glBindVertexArray(m_VAO_Fbo[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_Texture[1]);
        GLint textureY = glGetUniformLocation(m_Program_Fbo_NV21, "s_textureY");
        GLint textureVU = glGetUniformLocation(m_Program_Fbo_NV21, "s_textureVU");
        glUniform1i(textureY, 0);
        glUniform1i(textureVU, 1);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo[0]);
        glUseProgram(m_Program_Fbo_RGB);
        glBindVertexArray(m_VAO_Fbo[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        GLint textureRGB = glGetUniformLocation(m_Program_Fbo_RGB, "s_textureRGB");
        glUniform1i(textureRGB, 0);
    }
//    onMatrix("vMatrix", 0.0f, 0.0f);
    onMatrix("vMatrix", m_ViewRot, m_ModelRot);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) 0);
    onFrameBufferUpdate();
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //normal
    glViewport(0, 0, m_Width_display, m_Height_display);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glUseProgram(m_Program);
    glBindVertexArray(m_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_Texture_Fbo[0]);
    GLint textureMap = glGetUniformLocation(m_Program, "s_TextureMap");
    glUniform1i(textureMap, 0);
    onMatrix("vMatrix", 0.0f, 0.0f);
//    onMatrix("vMatrix", m_ViewRot, m_ModelRot);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) 0);
}

void VideoRender::onResume() {
}

void VideoRender::onPause() {
}

void VideoRender::onStop() {
}

void VideoRender::onRelease() {
    if (m_VBO[0]) {
        glDeleteBuffers(3, m_VBO);
    }
    if (m_Texture_Fbo[0]) {
        glDeleteTextures(1, m_Texture_Fbo);
    }
    if (m_VAO_Fbo[0]) {
        glDeleteVertexArrays(1, m_VAO_Fbo);
    }
    if (m_Fbo[0]) {
        glDeleteFramebuffers(1, m_Fbo);
    }
    if (m_Program_Fbo_YUV420P) {
        glDeleteProgram(m_Program_Fbo_YUV420P);
        m_Program_Fbo_YUV420P = GL_NONE;
    }
    if (m_Program_Fbo_NV21) {
        glDeleteProgram(m_Program_Fbo_NV21);
        m_Program_Fbo_NV21 = GL_NONE;
    }
    if (m_Program_Fbo_RGB) {
        glDeleteProgram(m_Program_Fbo_RGB);
        m_Program_Fbo_RGB = GL_NONE;
    }
    if (m_Texture[0]) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(3, m_Texture);
    }
    if (m_VAO[0]) {
        glDeleteVertexArrays(1, m_VAO);
    }
    if (m_Program) {
        glDeleteProgram(m_Program);
    }
    if (m_Image) {
        std::lock_guard<std::mutex> lock(m_Mutex);//加锁
        PixImageUtils::pix_image_free(m_Image);
    }
    m_Interrupt = true;
}

void VideoRender::onFrameBufferUpdate() {
    std::lock_guard<std::mutex> lock(m_Mutex);//加锁
    int width = m_Image->width;
    int height = m_Image->height;
    if (!m_data) m_data = new uint8_t[width * height * 4];
    m_dataSize = width * height * 4;
    if (m_CameraData) {
        glReadPixels(0, 0, height, width, GL_RGBA, GL_UNSIGNED_BYTE, m_data);
        if (m_RenderFrameCallback && m_CallbackContext) {
            m_RenderFrameCallback(m_CallbackContext, IMAGE_FORMAT_RGBA, height, width, m_data);
        }
    } else {
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, m_data);
        if (m_RenderFrameCallback && m_CallbackContext) {
            m_RenderFrameCallback(m_CallbackContext, IMAGE_FORMAT_RGBA, width, height, m_data);
        }
    }
}

}