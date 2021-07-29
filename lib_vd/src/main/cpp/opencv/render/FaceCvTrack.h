#ifndef VDMAKE_FACECVTRACK_H
#define VDMAKE_FACECVTRACK_H

#include "Log.h"
#include "opencv2/opencv.hpp"
#include "face_alignment.h"
#include <vector>

using namespace cv;
using namespace std;

class CascadeDetectorAdapter : public DetectionBasedTracker::IDetector {
public:
    CascadeDetectorAdapter(cv::Ptr<cv::CascadeClassifier> detector) :
            IDetector(),
            Detector(detector) {
        CV_Assert(detector);
    }

    void detect(const cv::Mat &Image, std::vector<cv::Rect> &objects) {
        Detector->detectMultiScale(Image, objects, scaleFactor, minNeighbours, 0, minObjSize,
                                   maxObjSize);
    }

    virtual ~CascadeDetectorAdapter() {
    }

private:
    cv::Ptr<cv::CascadeClassifier> Detector;
};

class FaceCvTrack {
public:
    void onModelSource(char *face, char *eye, char *nose, char *mouth, char *alignment);

    void onFacesTrack(int format, int width, int height, uint8_t *data,
                      std::vector<cv::Rect> &m_faces,
                      std::vector<cv::Rect> &m_eyes,
                      std::vector<cv::Rect> &m_noses,
                      std::vector<cv::Rect> &m_mouths);

private:
    Ptr<DetectionBasedTracker> m_Tracker;
    char *face_model = nullptr;
    char *eye_model = nullptr;
    char *nose_model = nullptr;
    char *mouth_model = nullptr;
    Ptr<seeta::FaceAlignment> m_Alignment;
    char *alignment_model = nullptr;
    cv::CascadeClassifier faceCascade;
};

#endif //VDMAKE_FACECVTRACK_H
