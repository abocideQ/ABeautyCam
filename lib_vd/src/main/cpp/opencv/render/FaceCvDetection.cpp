#include "FaceCvDetection.h"

extern "C" {
void FaceCvDetection::onModelSource(char *face, char *eye, char *nose, char *mouth) {
    face_model = face;
    eye_model = eye;
    nose_model = nose;
    mouth_model = mouth;
}

void FaceCvDetection::onFacesDetection(int format, int width, int height, uint8_t *data,
                                       std::vector<cv::Rect> &m_faces,
                                       std::vector<cv::Rect> &m_eyes,
                                       std::vector<cv::Rect> &m_noses,
                                       std::vector<cv::Rect> &m_mouths) {
    cv::Mat gray;
    if (format == 1) {
        cv::Mat src(height + height / 2, width, CV_8UC1, data);//yuv数据
        cv::cvtColor(src, gray, cv::COLOR_YUV2RGB_I420);//转RGB
        src.release();
    } else if (format == 2 || format == 3) {
        cv::Mat src(height + height / 2, width, CV_8UC2, data);//nv
        cv::cvtColor(src, gray, cv::COLOR_YUV2RGB_NV21);
        src.release();
    } else if (format == 4) {
        cv::Mat src(height + height, width, CV_8UC3, data);//rgb
        cv::cvtColor(src, gray, cv::COLOR_RGBA2RGB);
        src.release();
    } else {
        return;
    }
    cv::cvtColor(gray, gray, cv::COLOR_RGB2GRAY);//灰度图提高检测速度
    cv::equalizeHist(gray, gray);//直方图均衡化(二值化)
    //face check
    m_faces.clear();
    m_eyes.clear();
    m_noses.clear();
    m_mouths.clear();
    std::vector<cv::Rect> faces;
    faceDetection(face_model, faces, gray);
    if (faces.empty()) {
        return;
    }
    m_faces.insert(m_faces.end(), faces.begin(), faces.end());
    //features check
    int i = 0;
    int size = (int) faces.size();
    for (; i < size; i++) {
        cv::Rect face = faces[i];
//        cv::rectangle(src, face, cv::Scalar(255, 0, 0), 2, 4, 0);//矩形绘制
        cv::Mat ROI = gray(cv::Rect(face.x, face.y, face.width, face.height));//特征检测
        //eyes
        std::vector<cv::Rect> eyes;
        eyesDetection(eye_model, eyes, gray);
        if (!eyes.empty()) {
            m_eyes.insert(m_eyes.end(), eyes.begin(), eyes.end());
        }
        //nose
//        std::vector<cv::Rect> noses;
//        noseDetection(nose_model, noses, ROI);
//        if (!noses.empty()) {
//            m_noses.insert(m_noses.end(), noses.begin(), noses.end());
//        }
        //mouth
//        std::vector<cv::Rect> mouths;
//        mouthDetection(mouth_model, mouths, ROI);
//        if (!mouths.empty()) {
//            m_mouths.insert(m_mouths.end(), mouths.begin(), mouths.end());
//        }
    }
    gray.release();
}

void FaceCvDetection::faceDetection(char *model, std::vector<cv::Rect> &faces, cv::Mat image) {
    if (!model) {
        LOGCATE("FaceCvDetection::faceDetection face_model null error");
        return;
    }
    if (faceCascade.empty()) {
        faceCascade.load(model);
    }
    faceCascade.detectMultiScale(image, faces, 1.1f, 3, 0, cv::Size(50, 50));
    return;
}

void FaceCvDetection::eyesDetection(char *model, std::vector<cv::Rect> &features, cv::Mat image) {
    if (!model) {
        LOGCATE("FaceCvDetection::featuresCheck model null error");
        return;
    }
    if (eyesCascade.empty()) {
        eyesCascade.load(model);
    }
    eyesCascade.detectMultiScale(image, features, 1.5f, 3, 0, cv::Size(30, 30));
}

void FaceCvDetection::noseDetection(char *model, std::vector<cv::Rect> &features, cv::Mat image) {
    if (!model) {
        LOGCATE("FaceCvDetection::featuresCheck model null error");
        return;
    }
    if (noseCascade.empty()) {
        noseCascade.load(model);
    }
    noseCascade.detectMultiScale(image, features, 1.5f, 3, 0, cv::Size(30, 30));
}

void FaceCvDetection::mouthDetection(char *model, std::vector<cv::Rect> &features, cv::Mat image) {
    if (!model) {
        LOGCATE("FaceCvDetection::featuresCheck model null error");
        return;
    }
    if (mouthCascade.empty()) {
        mouthCascade.load(model);
    }
    mouthCascade.detectMultiScale(image, features, 1.5f, 3, 0, cv::Size(30, 30));
}
}






