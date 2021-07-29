#include "FaceCvTrack.h"

extern "C" {
void FaceCvTrack::onModelSource(char *face, char *eye, char *nose, char *mouth, char *alignment) {
    face_model = face;
    eye_model = eye;
    nose_model = nose;
    mouth_model = mouth;
    alignment_model = alignment;
    //创建主适配器
    Ptr<CascadeClassifier> faceCascade = makePtr<CascadeClassifier>(face_model);
    Ptr<CascadeDetectorAdapter> faceDetector = makePtr<CascadeDetectorAdapter>(faceCascade);
    //创建追踪检测适配器
    Ptr<CascadeClassifier> trackCascade = makePtr<CascadeClassifier>(face_model);
    Ptr<CascadeDetectorAdapter> trackDetector = makePtr<CascadeDetectorAdapter>(trackCascade);
    //创建追踪器
    DetectionBasedTracker::Parameters detectorParams;
    m_Tracker = makePtr<DetectionBasedTracker>(faceDetector, trackDetector, detectorParams);
    //启动追踪器
    m_Tracker->run();
    m_Alignment = makePtr<seeta::FaceAlignment>(alignment_model);
}

void FaceCvTrack::onFacesTrack(int format, int width, int height, uint8_t *data,
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
        cv::Mat src(height + height / 2, width, CV_8UC1, data);//nv
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
    cv::equalizeHist(gray, gray);//直方图均衡化(二值化)，增强对比度
    m_faces.clear();
    m_eyes.clear();
    m_noses.clear();
    m_mouths.clear();
    //检测
    m_Tracker->process(gray);
    //结果
    vector<Rect> faces;
    m_Tracker->getObjects(faces);
    if (faces.empty()) {
        return;
    }
    m_faces.insert(m_faces.end(), faces.begin(), faces.end());
    seeta::FacialLandmark landmark[5];
    int i = 0;
    int size = (int) faces.size();
    for (; i < size; i++) {
        cv::Rect face = faces[i];
        //faceinfo
        seeta::FaceInfo faceInfo;
        seeta::Rect bbox;
        bbox.x = face.x;
        bbox.y = face.y;
        bbox.width = face.width;
        bbox.height = face.height;
        faceInfo.bbox = bbox;
        //imagedata
        seeta::ImageData gray_im(gray.cols, gray.rows);
        gray_im.data = gray.data;
        //landmark
        m_Alignment->PointDetectLandmarks(gray_im, faceInfo, landmark);
        cv::Rect eyeL(landmark[0].x, landmark[0].y, 10.0f, 10.0f);
        cv::Rect eyeR(landmark[1].x, landmark[1].y, 10.0f, 10.0f);
        m_eyes.push_back(eyeL);
        m_eyes.push_back(eyeR);
        return;
    }
    gray.release();
}
}