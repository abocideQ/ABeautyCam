#ifndef VDMAKE_FACECNNDETECTION_H
#define VDMAKE_FACECNNDETECTION_H


#include "opencv2/opencv.hpp"
#include "facedetectcnn.h"
#include "face_alignment.h"

#include <vector>
#include "Log.h"

using namespace cv;
using namespace std;

class FaceCnnDetection {
public:
    void onModelSource(char *alignment);

    void onFacesDetection(int format, int width, int height, uint8_t *data, int cId,
                          std::vector<cv::Rect> &m_faces,
                          std::vector<cv::Rect> &m_eyes,
                          std::vector<cv::Rect> &m_noses,
                          std::vector<cv::Rect> &m_mouths);

protected:
    int timer = 0;
    std::vector<cv::Rect> faces;
    std::vector<cv::Rect> eyes;
    std::vector<cv::Rect> noses;
    std::vector<cv::Rect> mouths;
    Ptr<seeta::FaceAlignment> m_Alignment;
    char *alignment_model = nullptr;
};

#endif //VDMAKE_FACECNNDETECTION_H
