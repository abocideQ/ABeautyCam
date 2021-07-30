#ifndef VDMAKE_FACECVDETECTION_H
#define VDMAKE_FACECVDETECTION_H

#include "Log.h"
#include "opencv2/opencv.hpp"
#include <vector>

class FaceCvDetection {
public:
    void onModelSource(char *face, char *eye, char *nose, char *mouth);

    void onFacesDetection(int format, int width, int height, uint8_t *data, int cId,
                          std::vector<cv::Rect> &m_faces,
                          std::vector<cv::Rect> &m_eyes,
                          std::vector<cv::Rect> &m_noses,
                          std::vector<cv::Rect> &m_mouths);

protected:
    void faceDetection(char *model, std::vector<cv::Rect> &faces, cv::Mat image);

    void eyesDetection(char *model, std::vector<cv::Rect> &features, cv::Mat image);

    void noseDetection(char *model, std::vector<cv::Rect> &features, cv::Mat image);

    void mouthDetection(char *model, std::vector<cv::Rect> &features, cv::Mat image);

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


#endif //VDMAKE_FACECVDETECTION_H
