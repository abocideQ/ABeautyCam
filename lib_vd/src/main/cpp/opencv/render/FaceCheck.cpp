#include "FaceCheck.h"

extern "C" {
void FaceCheck::onModelSource(char *face, char *eye, char *nose, char *mouth) {
    face_model = face;
    eye_model = eye;
    nose_model = nose;
    mouth_model = mouth;
}

void *FaceCheck::onFaces(int width, int height, uint8_t *data, std::vector<cv::Rect> &m_faces,
                         std::vector<cv::Rect> &m_eyes, std::vector<cv::Rect> &m_noses,
                         std::vector<cv::Rect> &m_mouths) {
    cv::Mat gray;
    cv::Mat src(height + height / 2, width, CV_8UC1, data);//yuv数据
//    cv::Mat src(height, width, CV_8UC3, data);//rgb数据
//    cv::cvtColor(src, src, cv::COLOR_YUV2RGBA_I420);//YUV转RGB
    cv::cvtColor(src, gray, cv::COLOR_YUV2GRAY_420);//灰度图提高检测速度
    cv::equalizeHist(gray, gray);//直方图均衡化(二值化)
    //face check
    std::vector<cv::Rect> faces;
    faceCheck(face_model, faces, gray);
    if (faces.empty()) {
        return 0;
    }
    m_faces.insert(m_faces.end(), faces.begin(), faces.end());
    //features check
    int i = 0;
    for (; i < faces.size(); i++) {
        cv::Rect face = faces[i];
//        cv::rectangle(src, face, cv::Scalar(255, 0, 0), 2, 4, 0);//矩形绘制
        cv::Mat ROI = gray(cv::Rect(face.x, face.y, face.width, face.height));//特征检测
        //eyes
        std::vector<cv::Rect> eyes;
        eyesCheck(eye_model, eyes, ROI);
        if (!eyes.empty()) {
            m_eyes.insert(m_eyes.end(), eyes.begin(), eyes.end());
        }
        //nose
        std::vector<cv::Rect> noses;
        noseCheck(nose_model, noses, ROI);
        if (!noses.empty()) {
            m_noses.insert(m_noses.end(), noses.begin(), noses.end());
        }
        //mouth
        std::vector<cv::Rect> mouths;
        mouthCheck(mouth_model, mouths, ROI);
        if (!mouths.empty()) {
            m_mouths.insert(m_mouths.end(), mouths.begin(), mouths.end());
        }
    }
    gray.release();
    src.release();
}

void FaceCheck::faceCheck(char *model, std::vector<cv::Rect> &faces, cv::Mat image) {
    if (!model) {
        LOGCATE("FaceCheck::faceCheck face_model null error");
        return;
    }
    if (faceCascade.empty()) {
        faceCascade.load(model);
    }
    faceCascade.detectMultiScale(image, faces, 1.1f, 5, 0);
    return;
}

void FaceCheck::eyesCheck(char *model, std::vector<cv::Rect> &features, cv::Mat image) {
    if (!model) {
        LOGCATE("FaceCheck::featuresCheck model null error");
        return;
    }
    if (eyesCascade.empty()) {
        eyesCascade.load(model);
    }
    eyesCascade.detectMultiScale(image, features, 1.20f, 3, 0, cv::Size(30, 30));
}

void FaceCheck::noseCheck(char *model, std::vector<cv::Rect> &features, cv::Mat image) {
    if (!model) {
        LOGCATE("FaceCheck::featuresCheck model null error");
        return;
    }
    if (noseCascade.empty()) {
        noseCascade.load(model);
    }
    noseCascade.detectMultiScale(image, features, 1.20f, 3, 0, cv::Size(30, 30), cv::Size());
}

void FaceCheck::mouthCheck(char *model, std::vector<cv::Rect> &features, cv::Mat image) {
    if (!model) {
        LOGCATE("FaceCheck::featuresCheck model null error");
        return;
    }
    if (mouthCascade.empty()) {
        mouthCascade.load(model);
    }
    mouthCascade.detectMultiScale(image, features, 1.20f, 3, 0, cv::Size(30, 30), cv::Size());
}
}






