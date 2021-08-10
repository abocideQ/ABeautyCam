#include "FaceCnnDetection.h"

#define DETECT_BUFFER_SIZE 0x20000
extern "C" {

void FaceCnnDetection::onModelSource(char *alignment) {
    m_Alignment = makePtr<seeta::FaceAlignment>(alignment);
}

void FaceCnnDetection::onFacesDetection(int format, int width, int height, uint8_t *data, int cId,
                                        std::vector<cv::Rect> &m_faces,
                                        std::vector<cv::Rect> &m_eyes,
                                        std::vector<cv::Rect> &m_noses,
                                        std::vector<cv::Rect> &m_mouths) {
    timer++;
    if (timer % 2 == 0) {
        m_faces.insert(m_faces.end(), faces.begin(), faces.end());
        m_eyes.insert(m_eyes.end(), eyes.begin(), eyes.end());
        m_noses.insert(m_noses.end(), noses.begin(), noses.end());
        m_mouths.insert(m_mouths.end(), mouths.begin(), mouths.end());
        return;
    }
    if (timer > 1000) { timer = 0; }
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
    float DETECT_SCALE = (float) width / 128.0f;
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
//    LOGCATE("faceCnn faces %d", num);
/*    int i = 0;
    for (; i < num; i++) {
        seeta::FacialLandmark landmark[5];
        short *p = ((short *) (pResults + 1)) + 142 * i;
        int face_x = p[1] * DETECT_SCALE;
        int face_y = p[2] * DETECT_SCALE;
        int face_w = p[3] * DETECT_SCALE;
        int face_h = p[4] * DETECT_SCALE;
        //faceinfo
        seeta::FaceInfo faceInfo;
        seeta::Rect_Alignment bbox;
        bbox.x = face_y;
        bbox.y = face_x;
        bbox.width = face_w;
        bbox.height = face_h;
        faceInfo.bbox = bbox;
        //imagedata
        cv::Mat gray;
        cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);
        seeta::ImageData_Alignment gray_im(gray.cols, gray.rows);
        gray_im.data = gray.data;
        //landmark
        m_Alignment->PointDetectLandmarks(gray_im, faceInfo, landmark);
        gray.release();
        if (cId == 1) {
            cv::Rect fface(width - face_y - face_w, face_x, face_h,
                           face_w);
            m_faces.push_back(fface);
            cv::Rect eyeL(width - landmark[0].y, landmark[0].x, 10.0f, 10.0f);
            cv::Rect eyeR(width - landmark[1].y, landmark[1].x, 10.0f, 10.0f);
            m_eyes.push_back(eyeL);
            m_eyes.push_back(eyeR);
            cv::Rect nose(width - landmark[2].y, landmark[2].x, 10.0f, 10.0f);
            m_noses.push_back(nose);
            cv::Rect mouthL(width - landmark[3].y, landmark[3].x, 10.0f, 10.0f);
            cv::Rect mouthR(width - landmark[4].y, landmark[4].x, 10.0f, 10.0f);
            m_mouths.push_back(mouthL);
            m_mouths.push_back(mouthR);
        } else if (cId == 2) {
            cv::Rect fface(face_y, face_x, face_h, face_w);
            m_faces.push_back(fface);
            cv::Rect eyeL(landmark[0].y, landmark[0].x, 10.0f, 10.0f);
            cv::Rect eyeR(landmark[1].y, landmark[1].x, 10.0f, 10.0f);
            m_eyes.push_back(eyeL);
            m_eyes.push_back(eyeR);
            cv::Rect nose(landmark[2].y, landmark[2].x, 10.0f, 10.0f);
            m_noses.push_back(nose);
            cv::Rect mouthL(landmark[3].y, landmark[3].x, 10.0f, 10.0f);
            cv::Rect mouthR(landmark[4].y, landmark[4].x, 10.0f, 10.0f);
            m_mouths.push_back(mouthL);
            m_mouths.push_back(mouthR);
        }
        return;
    }*/
    int i = 0;
    for (; i < num; i++) {
        short *p = ((short *) (pResults + 1)) + 142 * i;
        int confidence = p[0];
        int face_x = p[1] * DETECT_SCALE;
        int face_y = p[2] * DETECT_SCALE;
        int face_w = p[3] * DETECT_SCALE;
        int face_h = p[4] * DETECT_SCALE;
        //??? = p[5/6/7/8]
        int mouthL_x = p[9] * DETECT_SCALE;
        int mouthL_y = p[10] * DETECT_SCALE;
        int mouthR_x = p[11] * DETECT_SCALE;
        int mouthR_y = p[12] * DETECT_SCALE;
        int eyeL_x = p[5] * DETECT_SCALE;
        int eyeL_y = p[6] * DETECT_SCALE;
        int eyeR_x = p[7] * DETECT_SCALE;
        int eyeR_y = p[8] * DETECT_SCALE;
        int nose_x = p[9] * DETECT_SCALE;
        int nose_y = p[10] * DETECT_SCALE;
        if (cId == 1) {
            cv::Rect fface(width - face_y - face_w, face_x, face_h, face_w);
            m_faces.push_back(fface);
            cv::Rect eyeL(width - eyeL_y, eyeL_x, 10.0f, 10.0f);
            cv::Rect eyeR(width - eyeR_y, eyeR_x, 10.0f, 10.0f);
            m_eyes.push_back(eyeL);
            m_eyes.push_back(eyeR);
            cv::Rect nose(width - nose_y, nose_x, 10.0f, 10.0f);
            m_noses.push_back(nose);
            cv::Rect mouthL(width - mouthL_y, mouthL_x, 10.0f, 10.0f);
            cv::Rect mouthR(width - mouthR_y, mouthR_x, 10.0f, 10.0f);
            m_mouths.push_back(mouthL);
            m_mouths.push_back(mouthR);
            faces.clear();
            eyes.clear();
            noses.clear();
            mouths.clear();
            faces.push_back(fface);
            eyes.push_back(eyeL);
            eyes.push_back(eyeR);
            noses.push_back(nose);
            mouths.push_back(mouthL);
            mouths.push_back(mouthR);
        } else if (cId == 2) {
            cv::Rect fface(face_y, face_x, face_h, face_w);
            m_faces.push_back(fface);
            cv::Rect eyeL(eyeL_y, eyeL_x, 10.0f, 10.0f);
            cv::Rect eyeR(eyeR_y, eyeR_x, 10.0f, 10.0f);
            m_eyes.push_back(eyeL);
            m_eyes.push_back(eyeR);
            cv::Rect nose(nose_y, nose_x, 10.0f, 10.0f);
            m_noses.push_back(nose);
            cv::Rect mouthL(mouthL_y, mouthL_x, 10.0f, 10.0f);
            cv::Rect mouthR(mouthR_y, mouthR_x, 10.0f, 10.0f);
            m_mouths.push_back(mouthL);
            m_mouths.push_back(mouthR);
            faces.clear();
            eyes.clear();
            noses.clear();
            mouths.clear();
            faces.push_back(fface);
            eyes.push_back(eyeL);
            eyes.push_back(eyeR);
            noses.push_back(nose);
            mouths.push_back(mouthL);
        }
        return;
    }
    free(pBuffer);
}
}

