#include "FaceCnnDetection.h"

#define DETECT_BUFFER_SIZE 0x20000
extern "C" {
void FaceCnnDetection::onFacesDetection(int format, int width, int height, uint8_t *data,
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
        cv::Mat src(height + height / 2, width, CV_8UC2, data);//nv
        cv::cvtColor(src, bgr, cv::COLOR_YUV2BGR_NV21);
        src.release();
    } else if (format == 4) {
        cv::Mat src(height + height, width, CV_8UC3, data);//rgb
        cv::cvtColor(src, bgr, cv::COLOR_RGBA2BGR);
        src.release();
    } else {
        return;
    }
    cv::resize(bgr, bgr, cv::Size(width / 5, height / 5));
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
    pResults = facedetect_cnn(pBuffer, (unsigned char *) (gray.ptr(0)), gray.cols, gray.rows,
                              (int) gray.step);
    int numFaces = pResults ? *pResults : 0;
    LOGCATE("onFacesDetection 5555555  === %d", numFaces);
    int i = 0;
    for (; i < (pResults ? *pResults : 0); i++) {
    }
}
}

