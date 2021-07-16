#ifndef VDMAKE_FACECHECK_H
#define VDMAKE_FACECHECK_H

#include "Log.h"
#include "PixImage.h"
#include "opencv2/opencv.hpp"
#include <vector>

class FaceCheck {
public:
    void onModelSource(char *face, char *eye, char *nose, char *mouth);

    void *onInsertFaces(int width, int height, uint8_t *data, PixImage *pix);

protected:
    void faceCheck(char *model, std::vector<cv::Rect> faces, cv::Mat image);

    void eyesCheck(char *model, std::vector<cv::Rect> features, cv::Mat image);

    void noseCheck(char *model, std::vector<cv::Rect> features, cv::Mat image);

    void mouthCheck(char *model, std::vector<cv::Rect> features, cv::Mat image);

private:
    char *face_model = nullptr;
    char *eye_model = nullptr;
    char *nose_model = nullptr;
    char *mouth_model = nullptr;

    cv::CascadeClassifier faceCascade;
    cv::CascadeClassifier eyesCascade;
    cv::CascadeClassifier noseCascade;
    cv::CascadeClassifier mouthCascade;
};


#endif //VDMAKE_FACECHECK_H
