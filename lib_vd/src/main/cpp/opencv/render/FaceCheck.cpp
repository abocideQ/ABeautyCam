#include "FaceCheck.h"

extern "C" {
void FaceCheck::onModelSource(char *face, char *eye, char *nose, char *mouth) {
    face_model = face;
    eye_model = eye;
    nose_model = nose;
    mouth_model = mouth;
}

void *FaceCheck::onInsertFaces(int width, int height, uint8_t *data, PixImage *pix) {
    cv::Mat gray;
    cv::Mat src(width, height, CV_8UC1, data, cv::Mat::AUTO_STEP);//源数据
    cv::cvtColor(src, src, cv::COLOR_YUV2RGBA_NV21, 0);//YUV转RGB
    cv::cvtColor(src, gray, cv::COLOR_RGBA2GRAY, 0);//灰度图提高检测速度
    cv::equalizeHist(gray, gray);//直方图均衡化(二值化)
    //face check
    std::vector<cv::Rect> faces;
    faceCheck(face_model, faces, gray);
    if (faces.empty()) {
        LOGCATE("FaceCheck::onBuffer faces empty ");
        return;
    }
    pix->faces.insert(pix->faces.size() - 1, faces[0], faces[faces.size() - 1]);
    //features check
    int i = 0;
    for (; i < faces.size(); i++) {
        cv::Rect face = faces[i];
//        cv::rectangle(src, face, Scalar(255, 0, 0), 2, 4, 0);//矩形绘制
        cv::Mat ROI = gray(cv::Rect(face.x, face.y, face.width, face.height));//特征检测
        //eyes
        std::vector<cv::Rect> eyes;
        featuresCheck(eye_model, eyes, ROI);
        if (!eyes.empty()) {
            pix->eyes.insert(pix->eyes.size() - 1, eyes[0], eyes[eyes.size() - 1]);
        }
        //nose
        std::vector<cv::Rect> noses;
        featuresCheck(nose_model, noses, ROI);
        if (!noses.empty()) {
            pix->noses.insert(pix->noses.size() - 1, noses[0], noses[noses.size() - 1]);
        }
        //mouth
        std::vector<cv::Rect> mouths;
        featuresCheck(mouth_model, mouths, ROI);
        if (!mouths.empty()) {
            pix->mouths.insert(pix->mouths.size() - 1, mouths[0], mouths[mouths.size() - 1]);
        }
    }
    gray.release();
    src.release();
}

void FaceCheck::faceCheck(char *model, std::vector<cv::Rect> faces, cv::Mat image) {
    if (model) {
        LOGCATE("FaceCheck::faceCheck face_model null error");
        return;
    }
    cv::CascadeClassifier faceCascade;//创建检测器
    faceCascade.load(model);
    if (faceCascade.empty()) {
        LOGCATE("FaceCheck::faceCheck faceCascade empty error");
        return;
    }
    faceCascade.detectMultiScale(image, faces, 1.1f, 3, 0, cv::Size(30, 30), cv::Size());

}

void FaceCheck::featuresCheck(char *model, std::vector<cv::Rect> features, cv::Mat image) {
    if (model) {
        LOGCATE("FaceCheck::featuresCheck model null error");
        return;
    }
    cv::CascadeClassifier featuresCascade;
    featuresCascade.load(model);
    if (featuresCascade.empty()) {
        LOGCATE("FaceCheck::featuresCheck featuresCascade empty error");
        return;
    }
    featuresCascade.detectMultiScale(image, features, 1.20f, 5, 0, cv::Size(30, 30), cv::Size());
}
}






