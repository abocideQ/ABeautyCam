#ifndef VDMAKE_FACECNNDETECTION_H
#define VDMAKE_FACECNNDETECTION_H

#include "Log.h"
#include "opencv2/opencv.hpp"
#include "facedetectcnn.h"
#include <vector>

class FaceCnnDetection {
public:
    void onFacesDetection(int format, int width, int height, uint8_t *data,
                          std::vector<cv::Rect> &m_faces,
                          std::vector<cv::Rect> &m_eyes,
                          std::vector<cv::Rect> &m_noses,
                          std::vector<cv::Rect> &m_mouths);
};

#endif //VDMAKE_FACECNNDETECTION_H
