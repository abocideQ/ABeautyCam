#include "FaceCnnDetection.h"

#define DETECT_BUFFER_SIZE 0x20000
#define DETECT_SCALE 10.0f
extern "C" {
void FaceCnnDetection::onFacesDetection(int format, int width, int height, uint8_t *data, int cId,
                                        std::vector<cv::Rect> &m_faces,
                                        std::vector<cv::Rect> &m_eyes,
                                        std::vector<cv::Rect> &m_noses,
                                        std::vector<cv::Rect> &m_mouths) {
    cv::Mat bgr;
    if (format == 1) {
        cv::Mat src(height + height / 2, width, CV_8UC1, data);//yuv数据
        cv::cvtColor(src, bgr, cv::COLOR_YUV2BGR_I420);//转BGR
        src.release();
    } else if (format == 2 || format == 3) {
        cv::Mat src(height + height / 2, width, CV_8UC1, data);//nv
        cv::cvtColor(src, bgr, cv::COLOR_YUV2BGR_NV21);
        src.release();
    } else if (format == 4) {
        cv::Mat src(height + height, width, CV_8UC3, data);//rgb
        cv::cvtColor(src, bgr, cv::COLOR_RGBA2BGR);
        src.release();
    } else {
        return;
    }
    cv::resize(bgr, bgr, cv::Size(width / DETECT_SCALE, height / DETECT_SCALE));
    if (cId == 1) {
        rotate(bgr, bgr, cv::ROTATE_90_COUNTERCLOCKWISE); //前置摄像头  逆时针旋转90度
    } else if (cId == 2) {
        rotate(bgr, bgr, cv::ROTATE_90_CLOCKWISE);
        flip(bgr, bgr, 1);//水平镜像  1：水平翻转；0：垂直翻转
    }
    m_faces.clear();
    m_eyes.clear();
    m_noses.clear();
    m_mouths.clear();
    int *pResults = NULL;
    unsigned char *pBuffer = (unsigned char *) malloc(DETECT_BUFFER_SIZE);
    if (!pBuffer) {
        LOGCATE("onFacesDetection DETECT_BUFFER_SIZE malloc error");
        return;
    }
    pResults = facedetect_cnn(pBuffer, (unsigned char *) (bgr.ptr(0)),
                              bgr.cols, bgr.rows, (int) bgr.step);
    int num = pResults ? *pResults : 0;
    LOGCATE("faceCnn faces %d", num);
    int i = 0;
    for (; i < (pResults ? *pResults : 0); i++) {
        short *p = ((short *) (pResults + 1)) + 142 * i;
        int confidence = p[0];
        int face_x = p[1] * DETECT_SCALE;
        int face_y = p[2] * DETECT_SCALE;
        int face_w = p[3] * DETECT_SCALE;
        int face_h = p[4] * DETECT_SCALE;
        //??? = p[5/6/7/8]
        int mouthL_x = p[5] * DETECT_SCALE;
        int mouthL_y = p[6] * DETECT_SCALE;
        int mouthR_x = p[7] * DETECT_SCALE;
        int mouthR_y = p[8] * DETECT_SCALE;
        int eyeL_x = p[9] * DETECT_SCALE;
        int eyeL_y = p[10] * DETECT_SCALE;
        int eyeR_x = p[11] * DETECT_SCALE;
        int eyeR_y = p[12] * DETECT_SCALE;
        int nose_x = p[13] * DETECT_SCALE;
        int nose_y = p[14] * DETECT_SCALE;
        cv::Rect face(face_x, face_y, face_w, face_h);
        m_faces.push_back(face);
        cv::Rect eyeL(face_x, face_y, 10.0f, 10.0f);
        cv::Rect eyeR(face_x + face_w, face_y, 10.0f, 10.0f);
        m_eyes.push_back(eyeL);
        m_eyes.push_back(eyeR);
        cv::Rect mouthL(face_x, face_y + face_h, 10.0f, 10.0f);
        cv::Rect mouthR(face_x + face_w, face_y + face_h, 10.0f, 10.0f);
        m_mouths.push_back(mouthL);
        m_mouths.push_back(mouthR);
        cv::Rect nose(nose_x, nose_y, 10.0f, 10.0f);
        m_noses.push_back(nose);
        return;
    }
    free(pBuffer);
}
}

