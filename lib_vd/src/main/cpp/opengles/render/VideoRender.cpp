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

void VideoRender::onFace(char *s1, char *s2, char *s3, char *s4, char *s5, int faceI) {
    m_Face = faceI;
    if (m_Face == -1) {
    } else if (m_Face == 1) { //opencv
        m_FaceCvDetection = new FaceCvDetection();
        m_FaceCvDetection->onModelSource(s1, s2, s3, s4);
    } else if (m_Face == 2) { //facecnn
        mFaceCnnDetection = new FaceCnnDetection();
    } else if (m_Face == 3) { //ncnn
        m_FaceNCNNDetection = new FaceNCNNDetection();
        m_FaceNCNNDetection->onModel(s1, s5, false);
    } else if (m_Face == 4) { //ncnn
        m_FaceTrack = new FaceCvTrack();
        m_FaceTrack->onModelSource(s1, s2, s3, s4, s5);
    }
}

void VideoRender::onBuffer(int format, int w, int h, int lineSize[3], uint8_t *data) {
    if (data == nullptr || format == 0 || w == 0 | h == 0) return;
    if (format == 1) {
        format = IMAGE_FORMAT_YUV420P;
    } else if (format == 2 || format == 3) {
        format = IMAGE_FORMAT_NV21;
    } else {
        format = IMAGE_FORMAT_RGBA;
    }
    PixImage *pixel;
    if (lineSize == nullptr) {
        pixel = PixImageUtils::pix_image_get(format, w, h, data);
    } else {
        pixel = PixImageUtils::pix_image_get(format, w, h, lineSize, &data);
    }
    int cId = 0;
    if (m_ModelRot == 0.0f) {
        cId = 2;
    } else {
        cId = 1;
    }
    std::vector<cv::Rect> faces;
    std::vector<cv::Rect> eyes;
    std::vector<cv::Rect> noses;
    std::vector<cv::Rect> mouths;
    if (m_Face == 1) {//opencv
        m_FaceCvDetection->onFacesDetection(format, w, h, data, cId, faces, eyes, noses, mouths);
    } else if (m_Face == 2) {//facecnn
        mFaceCnnDetection->onFacesDetection(format, w, h, data, faces, eyes, noses, mouths);
    } else if (m_Face == 3) {//ncnn
        m_FaceNCNNDetection->onDetect(format, w, h, data, faces, eyes, noses, mouths);
    } else if (m_Face == 4) {//cvTrack
        m_FaceTrack->onFacesTrack(format, w, h, data, cId, faces, eyes, noses, mouths);
    }
    VRender *render = new VRender();
    render->pixel = pixel;
    render->faces = faces;
    render->eyes = eyes;
    render->noses = noses;
    render->mouths = mouths;
    m_VRenderQueue.push(render);
}

void VideoRender::onBuffer(PixImage *pix) {
    if (pix == nullptr || pix->format == 0 || pix->width == 0 | pix->height == 0) return;
    VRender *render = new VRender();
    render->pixel = pix;
    m_VRenderQueue.push(render);
}

uint8_t *VideoRender::onFrameBuffer() {
    return m_FrameBuffer;
}

int VideoRender::onFrameBufferSize() {
    return m_FrameBufferSize;
}

void VideoRender::onCamera(bool camera) {
    m_CameraData = camera;
}

void VideoRender::onRotate(float viewRot, int modelRot) {
    m_ViewRot = viewRot;
    m_ModelRot = modelRot;
    std::lock_guard<std::mutex> lock(m_Mutex);//加锁
    onReleaseRenders();
}

void VideoRender::onSurfaceCreated() {
    m_Program = GLUtils::glProgram(ShaderVertex, ShaderFragment);
    m_Program_Fbo_YUV420P = GLUtils::glProgram(ShaderVertex_FBO, ShaderFragment_FBO_YUV420p);
    m_Program_Fbo_NV21 = GLUtils::glProgram(ShaderVertex_FBO, ShaderFragment_FBO_NV21);
    m_Program_Fbo_RGB = GLUtils::glProgram(ShaderVertex_FBO, ShaderFragment_FBO_RGB);
    m_Program_Fbo_NV21_Face = GLUtils::glProgram(ShaderVertex_FBO, ShaderFragment_FBO_NV21_Face);
    if (m_Program == GL_NONE) return;
    if (m_Program_Fbo_YUV420P == GL_NONE) return;
    if (m_Program_Fbo_NV21 == GL_NONE) return;
    if (m_Program_Fbo_RGB == GL_NONE) return;
    if (m_Program_Fbo_NV21_Face == GL_NONE) return;
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
    if (m_Program_Fbo_NV21_Face == GL_NONE) return;
    if (m_VRenderQueue.empty())return;
    //textureImage2d
    std::unique_lock<std::mutex> lock(m_Mutex);
    VRender *render = m_VRenderQueue.front();
    int format = render->pixel->format;
    int width = render->pixel->width;
    int height = render->pixel->height;
    PixImage *image = render->pixel;
    std::vector<cv::Rect> faces = render->faces;
    std::vector<cv::Rect> eyes = render->eyes;
    std::vector<cv::Rect> noses = render->noses;
    std::vector<cv::Rect> mouths = render->mouths;
    if (format == IMAGE_FORMAT_YUV420P) {
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, image->plane[0]);
        glBindTexture(GL_TEXTURE_2D, m_Texture[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, image->plane[1]);
        glBindTexture(GL_TEXTURE_2D, m_Texture[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, image->plane[2]);
    } else if (format == IMAGE_FORMAT_NV21 || format == IMAGE_FORMAT_NV12) {
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, image->plane[0]);
        glBindTexture(GL_TEXTURE_2D, m_Texture[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width / 2, height / 2,
                     0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, image->plane[1]);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, image->plane[0]);
    }
    if (m_VRenderQueue.size() > 2) {
        m_VRenderQueue.pop();
        onReleaseRender(render);
    }
    //offscreen
    if (m_CameraData) {
        glBindTexture(GL_TEXTURE_2D, m_Texture_Fbo[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, height, width, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glViewport(0, 0, height, width);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_Texture_Fbo[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glViewport(0, 0, width, height);
    }
    if (m_Face > 0 && format == IMAGE_FORMAT_NV21) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo[0]);
        glUseProgram(m_Program_Fbo_NV21_Face);
        glBindVertexArray(m_VAO_Fbo[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_Texture[1]);
        GLint textureY = glGetUniformLocation(m_Program_Fbo_NV21_Face, "s_textureY");
        GLint textureVU = glGetUniformLocation(m_Program_Fbo_NV21_Face, "s_textureVU");
        glUniform1i(textureY, 0);
        glUniform1i(textureVU, 1);
        GLfloat size = glGetUniformLocation(m_Program_Fbo_NV21_Face, "fPixelSize");
        glUniform2f(size, width, height);
        GLfloat facePoint = glGetUniformLocation(m_Program_Fbo_NV21_Face, "fFacePoint");
        GLfloat faceSize = glGetUniformLocation(m_Program_Fbo_NV21_Face, "fFaceSize");
        glUniform2f(facePoint, 0.0f, 0.0f);
        glUniform2f(faceSize, 0.0f, 0.0f);
        GLfloat eyeScale = glGetUniformLocation(m_Program_Fbo_NV21_Face, "fEyeScale");
        GLfloat eyeRadius = glGetUniformLocation(m_Program_Fbo_NV21_Face, "fEyeRadius");
        GLfloat eyeLeft = glGetUniformLocation(m_Program_Fbo_NV21_Face, "fEyeLeft");
        GLfloat eyeRight = glGetUniformLocation(m_Program_Fbo_NV21_Face, "fEyeRight");
        GLfloat nose = glGetUniformLocation(m_Program_Fbo_NV21_Face, "fNose");
        GLfloat mouthL = glGetUniformLocation(m_Program_Fbo_NV21_Face, "fMouthL");
        GLfloat mouthR = glGetUniformLocation(m_Program_Fbo_NV21_Face, "fMouthR");
        glUniform2f(eyeLeft, 0.0f, 0.0f);
        glUniform2f(eyeRight, 0.0f, 0.0f);
        glUniform2f(nose, 0.0f, 0.0f);
        glUniform2f(mouthL, 0.0f, 0.0f);
        glUniform2f(mouthR, 0.0f, 0.0f);
        if (!faces.empty()) {
            glUniform2f(facePoint, faces[0].x, faces[0].y);
            glUniform2f(faceSize, faces[0].width, faces[0].height);
            if (!eyes.empty()) {
                glUniform1f(eyeScale, 2.0f);
                glUniform1f(eyeRadius, faces[0].width / 10.0f);
                float offset = 0.0f;
                if (m_ModelRot == 0.0f) {
                    offset = -1.0f;
                } else {
                    offset = 1.0f;
                }
                glUniform2f(eyeLeft, eyes[0].x + offset, eyes[0].y);
                glUniform2f(eyeRight, eyes[1].x + offset, eyes[1].y);
            }
            if (!noses.empty()) {
                glUniform2f(nose, noses[0].x, noses[0].y);
            }
            if (!mouths.empty()) {
                glUniform2f(mouthL, mouths[0].x, mouths[0].y);
                glUniform2f(mouthR, mouths[1].x, mouths[1].y);
            }
        }
    } else if (format == IMAGE_FORMAT_YUV420P) {
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
    } else if (format == IMAGE_FORMAT_NV21 || format == IMAGE_FORMAT_NV12) {
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
    onFrameBufferUpdate(width, height);
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
    lock.unlock();
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
    if (m_Program_Fbo_NV21_Face) {
        glDeleteProgram(m_Program_Fbo_NV21_Face);
        m_Program_Fbo_NV21_Face = GL_NONE;
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
    onReleaseRenders();
    m_Interrupt = true;
}

void VideoRender::onFrameBufferUpdate(int width, int height) {
    if (!m_FrameBuffer) m_FrameBuffer = new uint8_t[width * height * 4];
    m_FrameBufferSize = width * height * 4;
    if (m_CameraData) {
        glReadPixels(0, 0, height, width, GL_RGBA, GL_UNSIGNED_BYTE, m_FrameBuffer);
        if (m_RenderFrameCallback && m_CallbackContext) {
            m_RenderFrameCallback(m_CallbackContext, IMAGE_FORMAT_RGBA, height, width,
                                  m_FrameBuffer);
        }
    } else {
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, m_FrameBuffer);
        if (m_RenderFrameCallback && m_CallbackContext) {
            m_RenderFrameCallback(m_CallbackContext, IMAGE_FORMAT_RGBA, width, height,
                                  m_FrameBuffer);
        }
    }
}

void VideoRender::onReleaseRenders() {
    while (!m_VRenderQueue.empty()) {
        VRender *render = m_VRenderQueue.front();
        m_VRenderQueue.pop();
        onReleaseRender(render);
    }
}

void VideoRender::onReleaseRender(VRender *render) {
    PixImageUtils::pix_image_free(render->pixel);
    if (!render->faces.empty()) render->faces.clear();
    if (!render->eyes.empty()) render->eyes.clear();
    if (!render->noses.empty()) render->noses.clear();
    if (!render->mouths.empty()) render->mouths.clear();
    delete render;
}
}


