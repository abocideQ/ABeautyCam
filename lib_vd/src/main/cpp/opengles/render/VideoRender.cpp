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

//初始化人脸
void VideoRender::onFace(char *s1, char *s2, char *s3, char *s4, char *s5, int faceI) {
    if (faceI == -1) {
    } else if (faceI == 1) { //opencv
        m_FaceCvDetection = new FaceCvDetection();
        m_FaceCvDetection->onModelSource(s1, s2, s3, s4);
    } else if (faceI == 2) { //opencvTrack
        m_FaceTrack = new FaceCvTrack();
        m_FaceTrack->onModelSource(s1, s2, s3, s4, s5);
    } else if (faceI == 3) { //faceCnn
        mFaceCnnDetection = new FaceCnnDetection();
        mFaceCnnDetection->onModelSource(s5);
    }
    m_FboMixProcess = 0.0f;
    FACE_ON = faceI;
}

//传入数据 方式1
void VideoRender::onBuffer(int format, int w, int h, int lineSize[3], uint8_t *data) {
    if (data == nullptr || format == 0 || w == 0 | h == 0) return;
    if (format == 1) {
        format = IMAGE_FORMAT_YUV420P;
    } else if (format == 2 || format == 3) {
        format = IMAGE_FORMAT_NV21;
    } else if (format == 4) {
        format = IMAGE_FORMAT_RGBA;
    }
    PixImage *pixel;
    if (lineSize == nullptr) {
        pixel = PixImageUtils::pix_image_get(format, w, h, data);
    } else {
        pixel = PixImageUtils::pix_image_get(format, w, h, lineSize, &data);
    }
    int cId;
    if (m_ModelRot == 0.0f) cId = 2;
    else cId = 1;
    std::vector<cv::Rect> faces;
    std::vector<cv::Rect> eyes;
    std::vector<cv::Rect> noses;
    std::vector<cv::Rect> mouths;
    if (FACE_ON == 1) {//opencv
        m_FaceCvDetection->onFacesDetection(format, w, h, data, cId, faces, eyes, noses, mouths);
    } else if (FACE_ON == 2) {//opencvTrack
        m_FaceTrack->onFacesTrack(format, w, h, data, cId, faces, eyes, noses, mouths);
    } else if (FACE_ON == 3) {//faceCnn
        mFaceCnnDetection->onFacesDetection(format, w, h, data, cId, faces, eyes, noses, mouths);
    }
    auto *render = new VRender();
    render->pixel = pixel;
    render->faces = faces;
    render->eyes = eyes;
    render->noses = noses;
    render->mouths = mouths;
    m_VRenderQueue.push(render);
}

//传入数据 方式2
void VideoRender::onBuffer(PixImage *pix) {
    if (pix == nullptr || pix->format == 0 || pix->width == 0 | pix->height == 0) return;
    auto *render = new VRender();
    render->pixel = pix;
    m_VRenderQueue.push(render);
}

//录制用数据
uint8_t *VideoRender::onFrameBuffer() {
    return m_FrameBuffer;
}

//录制用数据大小
int VideoRender::onFrameBufferSize() {
    return m_FrameBufferSize;
}

//是否是摄像头数据
void VideoRender::onCamera(bool camera) {
    m_CameraData = camera;
}

//旋转 镜像
void VideoRender::onRotate(float viewRot, float modelRot) {
    m_ViewRot = viewRot;
    m_ModelRot = modelRot;
    std::lock_guard<std::mutex> lock(m_Mutex);//加锁
    onReleaseRenders();
}

//gl初始化
void VideoRender::onSurfaceCreated() {
    m_Program = GLUtils::glProgram(ShaderVertex, ShaderFragment);
    if (m_Program == GL_NONE) return;
    m_Fbo_Program[0] = GLUtils::glProgram(ShaderVertex_FBO, ShaderFragment_FBO_YUV420p_Display);
    m_Fbo_Program[1] = GLUtils::glProgram(ShaderVertex_FBO, ShaderFragment_FBO_NV21_Display);
    m_Fbo_Program[2] = GLUtils::glProgram(ShaderVertex_FBO, ShaderFragment_FBO_RGB_Display);
    if (m_Fbo_Program[0] == GL_NONE) return;
    if (m_Fbo_Program[1] == GL_NONE) return;
    if (m_Fbo_Program[2] == GL_NONE) return;
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
    //offscreen 展示 vao/texture/fbo初始化
    glGenVertexArrays(1, m_Fbo_VAO);
    glBindVertexArray(m_Fbo_VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *) nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[2]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) nullptr);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBO[3]);

    glGenTextures(1, m_Fbo_Texture);
    glBindTexture(GL_TEXTURE_2D, m_Fbo_Texture[0]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenFramebuffers(1, m_Fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo[0]);
    glBindTexture(GL_TEXTURE_2D, m_Fbo_Texture[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Fbo_Texture[0],
                           0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGCATE("gl_error::CreateFrameBufferObj status != GL_FRAMEBUFFER_COMPLETE");
    }
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    GL_ERROR_CHECK();
    //normal 展示 vao/texture初始化
    glGenVertexArrays(1, m_VAO);
    glBindVertexArray(m_VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *) nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) nullptr);
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
    //离屏处理 FBO 初始化
    onFrameMixCreated();
    //离屏输出 FBO 初始化
    onFrameOutCreated();
    //pbo
    glGenBuffers(6, m_PBO_In);
    glGenBuffers(2, m_PBO_Out);
    GL_ERROR_CHECK();
}

//fbo 离屏处理 初始化
void VideoRender::onFrameMixCreated() {
    const char *shaders[11] = {
            ShaderFragment_FBO_NV212RGB,
            ShaderFragment_FBO_BigEye,
            ShaderFragment_FBO_GaussBlurAWay,
            ShaderFragment_FBO_GaussBlurAWay,
            ShaderFragment_FBO_HighPassGauss,
            ShaderFragment_FBO_GaussBlurAWay,
            ShaderFragment_FBO_GaussBlurAWay,
            ShaderFragment_FBO_Beauty,
            ShaderFragment_FBO_Sharpen,
            ShaderFragment_FBO_Smooth,
            ShaderFragment_FBO_Transition
    };
    for (auto &shader : shaders) {
        FrameBufferObj obj;
        obj.shader = shader;
        obj.m_FboMix_Program[0] = GLUtils::glProgram(ShaderVertex_FBO, obj.shader);
        if (obj.m_FboMix_Program[0] == GL_NONE) return;
        //vao
        glGenVertexArrays(1, obj.m_FboMix_VAO);
        glBindVertexArray(obj.m_FboMix_VAO[0]);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *) nullptr);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO[2]);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) nullptr);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBO[3]);
        //texture
        glGenTextures(1, obj.m_FboMix_Texture);
        glBindTexture(GL_TEXTURE_2D, obj.m_FboMix_Texture[0]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //fbo + colorAttch
        glGenFramebuffers(1, obj.m_FboMix);
        glBindFramebuffer(GL_FRAMEBUFFER, obj.m_FboMix[0]);
        glBindTexture(GL_TEXTURE_2D, obj.m_FboMix_Texture[0]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               obj.m_FboMix_Texture[0],
                               0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOGCATE("gl_error::CreateFrameBufferObj status != GL_FRAMEBUFFER_COMPLETE");
        }
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
        m_FboMixes.push_back(obj);
        GL_ERROR_CHECK();
    }
}

void VideoRender::onFrameOutCreated() {
    m_FboOut_Program[0] = GLUtils::glProgram(ShaderVertex_FBO, ShaderFragment_FBO_RGB2NV21);
    if (m_FboOut_Program[0] == GL_NONE) return;
    //vao
    glGenVertexArrays(1, m_FboOut_VAO);
    glBindVertexArray(m_FboOut_VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *) nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[2]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) nullptr);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBO[3]);
    //texture
    glGenTextures(1, m_FboOut_Texture);
    glBindTexture(GL_TEXTURE_2D, m_FboOut_Texture[0]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //fbo + colorAttch
    glGenFramebuffers(1, m_FboOut);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboOut[0]);
    glBindTexture(GL_TEXTURE_2D, m_FboOut_Texture[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FboOut_Texture[0],
                           0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGCATE("gl_error::CreateFrameBufferObj status != GL_FRAMEBUFFER_COMPLETE");
    }
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    GL_ERROR_CHECK();
}

//surface高宽
void VideoRender::onSurfaceChanged(int width, int height) {
    m_Width_display = width;
    m_Height_display = height;
}

//摄像头
void VideoRender::onMatrix(const char *gl_name, float viewRot, float modelRot) const {
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

//gl绘制
void VideoRender::onDrawFrame() {
    if (m_Program == GL_NONE) return;
    if (m_Fbo_Program[0] == GL_NONE) return;
    if (m_Fbo_Program[1] == GL_NONE) return;
    if (m_Fbo_Program[2] == GL_NONE) return;
    //数据
    if (m_VRenderQueue.empty())return;
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
    if (m_PBO && (format == IMAGE_FORMAT_NV21 || format == IMAGE_FORMAT_NV12)) {
        //y
        int oddIndex = m_PBO_Index % 2;
        m_PBO_Index++;
        int index = oddIndex;
        int nextIndex = (index + 1) % 2;
        int size = width * height;
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_PBO_In[index]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE,
                     GL_UNSIGNED_BYTE, nullptr);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_PBO_In[nextIndex]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, GL_STREAM_DRAW);
        auto *ptr1 = (GLubyte *) glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0,
                                                  size,
                                                  GL_MAP_WRITE_BIT |
                                                  GL_MAP_INVALIDATE_BUFFER_BIT);
        if (ptr1) {
            memcpy(ptr1, image->plane[0], static_cast<size_t>(size));
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        //uv
        size = width * height / 2;
        index = oddIndex + 2;
        nextIndex = (index + 1) % 2 + 2;
        glBindTexture(GL_TEXTURE_2D, m_Texture[1]);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_PBO_In[index]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width / 2, height / 2, 0,
                     GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, nullptr);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_PBO_In[nextIndex]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, GL_STREAM_DRAW);
        auto *ptr2 = (GLubyte *) glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0,
                                                  size,
                                                  GL_MAP_WRITE_BIT |
                                                  GL_MAP_INVALIDATE_BUFFER_BIT);
        if (ptr2) {
            memcpy(ptr2, image->plane[1], static_cast<size_t>(size));
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        if (m_PBO_Index < 2) return; //防绿屏
    } else if (format == IMAGE_FORMAT_YUV420P) {
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
    } else if (format == IMAGE_FORMAT_RGBA) {
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, image->plane[0]);
    }
    if (m_VRenderQueue.size() > 2) {
        m_VRenderQueue.pop();
        onReleaseRender(render);
    }
    //offscreen 离屏纹理高宽
    if (m_CameraData) {
        glBindTexture(GL_TEXTURE_2D, m_Fbo_Texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, height, width, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glViewport(0, 0, height, width);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_Fbo_Texture[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glViewport(0, 0, width, height);
    }
    if (FACE_ON && format == IMAGE_FORMAT_NV21) {
        GLuint texture = onDrawFrameMix(width, height, faces, eyes, noses, mouths);
        //rgb
        glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo[0]);
        glUseProgram(m_Fbo_Program[2]);
        glBindVertexArray(m_Fbo_VAO[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        GLint textureRGB = glGetUniformLocation(m_Fbo_Program[2], "s_textureRGB");
        glUniform1i(textureRGB, 0);
    } else if (format == IMAGE_FORMAT_YUV420P) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo[0]);
        glUseProgram(m_Fbo_Program[0]);
        glBindVertexArray(m_Fbo_VAO[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_Texture[1]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_Texture[2]);
        GLint textureY = glGetUniformLocation(m_Fbo_Program[0], "s_textureY");
        GLint textureU = glGetUniformLocation(m_Fbo_Program[0], "s_textureU");
        GLint textureV = glGetUniformLocation(m_Fbo_Program[0], "s_textureV");
        glUniform1i(textureY, 0);
        glUniform1i(textureU, 1);
        glUniform1i(textureV, 2);
    } else if (format == IMAGE_FORMAT_NV21 || format == IMAGE_FORMAT_NV12) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo[0]);
        glUseProgram(m_Fbo_Program[1]);
        glBindVertexArray(m_Fbo_VAO[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_Texture[1]);
        GLint textureY = glGetUniformLocation(m_Fbo_Program[1], "s_textureY");
        GLint textureVU = glGetUniformLocation(m_Fbo_Program[1], "s_textureVU");
        glUniform1i(textureY, 0);
        glUniform1i(textureVU, 1);
    } else if (format == IMAGE_FORMAT_RGBA) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo[0]);
        glUseProgram(m_Fbo_Program[2]);
        glBindVertexArray(m_Fbo_VAO[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
        GLint textureRGB = glGetUniformLocation(m_Fbo_Program[2], "s_textureRGB");
        glUniform1i(textureRGB, 0);
    } else {
        return;
    }
//    onMatrix("vMatrix", 0.0f, 0.0f);
    onMatrix("vMatrix", m_ViewRot, m_ModelRot);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    if (!m_FboOutYUV) onFrameBufferUpdate(width, height);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //FrameBuffer Out
    if (m_FboOutYUV) onFrameBufferUpdate(width, height);
    //normal 绘制
    glViewport(0, 0, m_Width_display, m_Height_display);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glUseProgram(m_Program);
    glBindVertexArray(m_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_Fbo_Texture[0]);
    GLint textureMap = glGetUniformLocation(m_Program, "s_TextureMap");
    glUniform1i(textureMap, 0);
    onMatrix("vMatrix", 0.0f, 0.0f);
//    onMatrix("vMatrix", m_ViewRot, m_ModelRot);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    lock.unlock();
}

//fbo 离屏处理
GLuint VideoRender::onDrawFrameMix(int width, int height,
                                   std::vector<cv::Rect> faces,
                                   std::vector<cv::Rect> eyes,
                                   std::vector<cv::Rect> noses,
                                   std::vector<cv::Rect> mouths) {
    //offscreen 离屏纹理高宽
    for (auto &fbo : m_FboMixes) {
        if (m_CameraData) {
            glBindTexture(GL_TEXTURE_2D, fbo.m_FboMix_Texture[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, height, width, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);
            glViewport(0, 0, height, width);
        } else {
            glBindTexture(GL_TEXTURE_2D, fbo.m_FboMix_Texture[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);
            glViewport(0, 0, width, height);
        }
    }
    GLuint inputTex;
    GLuint gaussTex;
    GLuint passGaussTex;
    //nv21 2 rgb
    FrameBufferObj obj = m_FboMixes[0];
    glBindFramebuffer(GL_FRAMEBUFFER, obj.m_FboMix[0]);
    glUseProgram(obj.m_FboMix_Program[0]);
    glBindVertexArray(obj.m_FboMix_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_Texture[1]);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "nv21Y"), 0);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "nv21VU"), 1);
    onMatrix("vMatrix", 0.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //big eye 大眼
    obj = m_FboMixes[1];
    glBindFramebuffer(GL_FRAMEBUFFER, obj.m_FboMix[0]);
    glUseProgram(obj.m_FboMix_Program[0]);
    glBindVertexArray(obj.m_FboMix_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FboMixes[0].m_FboMix_Texture[0]);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureRgb"), 0);
    glUniform2f(glGetUniformLocation(obj.m_FboMix_Program[0], "fPixelSize"), width, height);
    GLfloat eyeScale = glGetUniformLocation(obj.m_FboMix_Program[0], "fEyeScale");
    GLfloat eyeRadius = glGetUniformLocation(obj.m_FboMix_Program[0], "fEyeRadius");
    GLfloat eyeLeft = glGetUniformLocation(obj.m_FboMix_Program[0], "fEyeLeft");
    GLfloat eyeRight = glGetUniformLocation(obj.m_FboMix_Program[0], "fEyeRight");
    GLfloat nose = glGetUniformLocation(obj.m_FboMix_Program[0], "fNose");
    GLfloat mouthL = glGetUniformLocation(obj.m_FboMix_Program[0], "fMouthL");
    GLfloat mouthR = glGetUniformLocation(obj.m_FboMix_Program[0], "fMouthR");
    glUniform2f(eyeLeft, 0.0f, 0.0f);
    glUniform2f(eyeRight, 0.0f, 0.0f);
    glUniform2f(nose, 0.0f, 0.0f);
    glUniform2f(mouthL, 0.0f, 0.0f);
    glUniform2f(mouthR, 0.0f, 0.0f);
    if (!faces.empty()) {
        GLfloat facePoint = glGetUniformLocation(obj.m_FboMix_Program[0], "fFacePoint");
        GLfloat faceSize = glGetUniformLocation(obj.m_FboMix_Program[0], "fFaceSize");
        glUniform2f(facePoint, faces[0].x, faces[0].y);
        glUniform2f(faceSize, faces[0].width, faces[0].height);
        if (!eyes.empty()) {
            glUniform1f(eyeScale, 0.1f);
            glUniform1f(eyeRadius, faces[0].width / 10.0f);
            float offset;
            if (m_ModelRot == 0.0f) offset = -1.0f;
            else offset = 1.0f;
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
    onMatrix("vMatrix", 0.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    inputTex = obj.m_FboMix_Texture[0];
    //gauss 横向高斯
    obj = m_FboMixes[2];
    glBindFramebuffer(GL_FRAMEBUFFER, obj.m_FboMix[0]);
    glUseProgram(obj.m_FboMix_Program[0]);
    glBindVertexArray(obj.m_FboMix_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTex);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureRGB"), 0);
    glUniform2f(glGetUniformLocation(obj.m_FboMix_Program[0], "fPixelSize"), width, 0.0f);
    onMatrix("vMatrix", 0.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //gauss 横向高斯
    obj = m_FboMixes[3];
    glBindFramebuffer(GL_FRAMEBUFFER, obj.m_FboMix[0]);
    glUseProgram(obj.m_FboMix_Program[0]);
    glBindVertexArray(obj.m_FboMix_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FboMixes[2].m_FboMix_Texture[0]);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureRGB"), 0);
    glUniform2f(glGetUniformLocation(obj.m_FboMix_Program[0], "fPixelSize"), 0.0f, height);
    onMatrix("vMatrix", 0.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    gaussTex = obj.m_FboMix_Texture[0];
    //highPass 高反差
    obj = m_FboMixes[4];
    glBindFramebuffer(GL_FRAMEBUFFER, obj.m_FboMix[0]);
    glUseProgram(obj.m_FboMix_Program[0]);
    glBindVertexArray(obj.m_FboMix_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gaussTex);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureRGB"), 0);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureGaussRGB"), 1);
    glUniform2f(glGetUniformLocation(obj.m_FboMix_Program[0], "fPixelSize"), width, height);
    onMatrix("vMatrix", 0.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //highPass -> Gauss 横向
    obj = m_FboMixes[5];
    glBindFramebuffer(GL_FRAMEBUFFER, obj.m_FboMix[0]);
    glUseProgram(obj.m_FboMix_Program[0]);
    glBindVertexArray(obj.m_FboMix_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FboMixes[4].m_FboMix_Texture[0]);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureRGB"), 0);
    glUniform2f(glGetUniformLocation(obj.m_FboMix_Program[0], "fPixelSize"), width, 0.0f);
    onMatrix("vMatrix", 0.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //highPass -> Gauss 竖向
    obj = m_FboMixes[6];
    glBindFramebuffer(GL_FRAMEBUFFER, obj.m_FboMix[0]);
    glUseProgram(obj.m_FboMix_Program[0]);
    glBindVertexArray(obj.m_FboMix_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FboMixes[5].m_FboMix_Texture[0]);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureRGB"), 0);
    glUniform2f(glGetUniformLocation(obj.m_FboMix_Program[0], "fPixelSize"), 0.0f, height);
    onMatrix("vMatrix", 0.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    passGaussTex = obj.m_FboMix_Texture[0];
    //beauty face 美颜
    obj = m_FboMixes[7];
    glBindFramebuffer(GL_FRAMEBUFFER, obj.m_FboMix[0]);
    glUseProgram(obj.m_FboMix_Program[0]);
    glBindVertexArray(obj.m_FboMix_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gaussTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, passGaussTex);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureSource"), 0);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureGauss"), 1);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "texturePassGauss"), 2);
    onMatrix("vMatrix", 0.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //sharpen 锐化
    obj = m_FboMixes[8];
    glBindFramebuffer(GL_FRAMEBUFFER, obj.m_FboMix[0]);
    glUseProgram(obj.m_FboMix_Program[0]);
    glBindVertexArray(obj.m_FboMix_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FboMixes[7].m_FboMix_Texture[0]);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureRGB"), 0);
    glUniform2f(glGetUniformLocation(obj.m_FboMix_Program[0], "fPixelSize"), width, height);
    onMatrix("vMatrix", 0.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //smooth 均值/中值/大小值 滤波
    obj = m_FboMixes[9];
    glBindFramebuffer(GL_FRAMEBUFFER, obj.m_FboMix[0]);
    glUseProgram(obj.m_FboMix_Program[0]);
    glBindVertexArray(obj.m_FboMix_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FboMixes[8].m_FboMix_Texture[0]);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureRGB"), 0);
    glUniform2f(glGetUniformLocation(obj.m_FboMix_Program[0], "fPixelSize"), width, height);
    onMatrix("vMatrix", 0.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //转场
    obj = m_FboMixes[10];
    glBindFramebuffer(GL_FRAMEBUFFER, obj.m_FboMix[0]);
    glUseProgram(obj.m_FboMix_Program[0]);
    glBindVertexArray(obj.m_FboMix_VAO[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_FboMixes[9].m_FboMix_Texture[0]);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureSource"), 0);
    glUniform1i(glGetUniformLocation(obj.m_FboMix_Program[0], "textureNext"), 1);
    if (m_FboMixProcess < 1.0f) {
        m_FboMixProcess += 0.03;
        glUniform1f(glGetUniformLocation(obj.m_FboMix_Program[0], "progress"), m_FboMixProcess);
    }
    onMatrix("vMatrix", 0.0f, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return obj.m_FboMix_Texture[0];
}

void VideoRender::onResume() {
}

void VideoRender::onPause() {
}

void VideoRender::onStop() {
}

void VideoRender::onRelease() {
    //fbo
    if (m_Fbo_VAO[0]) {
        glDeleteVertexArrays(1, m_Fbo_VAO);
    }
    if (m_Fbo_Texture[0]) {
        glDeleteTextures(1, m_Fbo_Texture);
    }
    if (m_Fbo[0]) {
        glDeleteFramebuffers(1, m_Fbo);
    }
    if (m_Fbo_Program[0]) {
        glDeleteProgram(m_Fbo_Program[0]);
        m_Fbo_Program[0] = GL_NONE;
    }
    if (m_Fbo_Program[1]) {
        glDeleteProgram(m_Fbo_Program[1]);
        m_Fbo_Program[1] = GL_NONE;
    }
    if (m_Fbo_Program[2]) {
        glDeleteProgram(m_Fbo_Program[2]);
        m_Fbo_Program[2] = GL_NONE;
    }
    //===
    if (m_VBO[0]) {
        glDeleteBuffers(3, m_VBO);
    }
    if (m_VAO[0]) {
        glDeleteVertexArrays(1, m_VAO);
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
    if (m_Program) {
        glDeleteProgram(m_Program);
    }
    //====renders
    onReleaseRenders();
    m_Interrupt = true;
}
//更新录制用数据
void VideoRender::onFrameBufferUpdate(int width, int height) {
    if (!m_RenderFrameCallback || !m_CallbackContext) return;
    //fbo rgb->nv21
    if (m_FboOutYUV) { //opengl RBG 转 YUYV(YUV422) 目前暂不支持此格式录制
        if (m_CameraData) {
            glBindTexture(GL_TEXTURE_2D, m_FboOut_Texture[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, height / 2, width, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);
            glViewport(0, 0, height / 2, width);
        } else {
            glBindTexture(GL_TEXTURE_2D, m_FboOut_Texture[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width / 2, height, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);
            glViewport(0, 0, width / 2, height);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, m_FboOut[0]);
        glUseProgram(m_FboOut_Program[0]);
        glBindVertexArray(m_FboOut_VAO[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_Fbo_Texture[0]);
        GLint textureRGB = glGetUniformLocation(m_FboOut_Program[0], "s_textureRGB");
        glUniform1i(textureRGB, 0);
        GLint offset = glGetUniformLocation(m_FboOut_Program[0], "u_Offset");
        glUniform1f(offset, 1.0f / (float) height);
        onMatrix("vMatrix", 0.0f, 0.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *) nullptr);
        //buffer
        if (!m_FrameBuffer) m_FrameBuffer = new uint8_t[width * height * 2];
        if (m_PBO && m_CameraData) {
            int oddIndex = (m_PBO_Index - 1) % 2;
            int size = width * height * 2;
            int index = oddIndex;
            int nextIndex = (index + 1) % 2;
            glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBO_Out[index]);
            glBufferData(GL_PIXEL_PACK_BUFFER, size, nullptr, GL_STREAM_READ);
            glReadPixels(0, 0, height / 2, width, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBO_Out[nextIndex]);
            glBufferData(GL_PIXEL_PACK_BUFFER, size, nullptr, GL_STREAM_READ);
            auto *ptrOut = (GLubyte *) glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0,
                                                        size,
                                                        GL_MAP_READ_BIT);
            if (ptrOut) {
                m_FrameBuffer = ptrOut;
                m_FrameBufferSize = size;
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            }
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            if (m_RenderFrameCallback && m_CallbackContext) {
                m_RenderFrameCallback(m_CallbackContext,
                                      IMAGE_FORMAT_NV21,
                                      height,
                                      width,
                                      m_FrameBuffer);
            }
        } else {
            m_FrameBufferSize = width * height * 2;
            if (m_CameraData) {
                glReadPixels(0, 0, height / 2, width, GL_RGBA, GL_UNSIGNED_BYTE, m_FrameBuffer);
                if (m_RenderFrameCallback && m_CallbackContext) {
                    m_RenderFrameCallback(m_CallbackContext, IMAGE_FORMAT_NV21, height, width,
                                          m_FrameBuffer);
                }
            } else {
                glReadPixels(0, 0, width / 2, height, GL_RGBA, GL_UNSIGNED_BYTE, m_FrameBuffer);
                if (m_RenderFrameCallback && m_CallbackContext) {
                    m_RenderFrameCallback(m_CallbackContext, IMAGE_FORMAT_NV21, width, height,
                                          m_FrameBuffer);
                }
            }
        }
        //fbo
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        if (!m_FrameBuffer) m_FrameBuffer = new uint8_t[width * height * 4];
        if (m_PBO && m_CameraData) {
            int oddIndex = (m_PBO_Index - 1) % 2;
            int size = width * height * 4;
            int index = oddIndex;
            int nextIndex = (index + 1) % 2;
            glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBO_Out[index]);
            glBufferData(GL_PIXEL_PACK_BUFFER, size, 0, GL_STREAM_READ);
            glReadPixels(0, 0, height, width, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBO_Out[nextIndex]);
            glBufferData(GL_PIXEL_PACK_BUFFER, size, 0, GL_STREAM_READ);
            auto *ptrOut = (GLubyte *) glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0,
                                                        size,
                                                        GL_MAP_READ_BIT);
            if (ptrOut) {
                m_FrameBuffer = ptrOut;
                m_FrameBufferSize = size;
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            }
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            if (m_RenderFrameCallback && m_CallbackContext) {
                m_RenderFrameCallback(m_CallbackContext, IMAGE_FORMAT_RGBA, height, width,
                                      m_FrameBuffer);
            }
        } else {
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
    }
}

//释放所有数据
void VideoRender::onReleaseRenders() {
    while (!m_VRenderQueue.empty()) {
        VRender *render = m_VRenderQueue.front();
        m_VRenderQueue.pop();
        onReleaseRender(render);
    }
}

//释放指定数据
void VideoRender::onReleaseRender(VRender *render) {
    PixImageUtils::pix_image_free(render->pixel);
    if (!render->faces.empty()) render->faces.clear();
    if (!render->eyes.empty()) render->eyes.clear();
    if (!render->noses.empty()) render->noses.clear();
    if (!render->mouths.empty()) render->mouths.clear();
    delete render;
}
}



