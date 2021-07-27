#include "FaceNCNNDetection.h"

#define DETECT_SCALE 3.0f
extern "C" {

int FaceNCNNDetection::onModel(char *folder, bool gpu) {
    m_DetectNet.clear();
    m_MeshNet.clear();
    m_SegNet.clear();
    ncnn::set_cpu_powersave(2);
    ncnn::set_omp_num_threads(ncnn::get_big_cpu_count());
    m_DetectNet.opt = ncnn::Option();
    m_MeshNet.opt = ncnn::Option();
    m_SegNet.opt = ncnn::Option();
#if NCNN_VULKAN
    m_DetectNet.opt.use_vulkan_compute = gpu;
    m_MeshNet.opt.use_vulkan_compute = gpu;
    m_SegNet.opt.use_vulkan_compute = gpu;
#endif
    m_DetectNet.opt.num_threads = ncnn::get_big_cpu_count();
    m_MeshNet.opt.num_threads = ncnn::get_big_cpu_count();
    m_SegNet.opt.num_threads = ncnn::get_big_cpu_count();
    const char *models[] = {
            "scrfd_1g-opt2.param",
            "scrfd_1g-opt2.bin",
            "facemesh-op.param",
            "facemesh-op.bin",
            "faceseg-op.param",
            "faceseg-op.bin",
    };
    m_Kps = false;
    int i = 0;
    for (; i < 6; i++) {
        std::string cc1 = folder;
        std::string cc2 = models[i];
        std::string cc3 = cc1 + cc2;
        LOGCATE("model init %s", cc3.c_str());
        if (i == 0) {
            m_DetectNet.load_param(cc3.c_str());
        } else if (i == 1) {
            m_DetectNet.load_model(cc3.c_str());
        } else if (i == 2) {
            m_MeshNet.load_param(cc3.c_str());
        } else if (i == 3) {
            m_MeshNet.load_model(cc3.c_str());
        } else if (i == 4) {
            m_SegNet.load_param(cc3.c_str());
        } else if (i == 5) {
            m_SegNet.load_model(cc3.c_str());
        }
    }
    return 0;
}

int FaceNCNNDetection::onDetect(int format, int w1, int h1, uint8_t *data,
                                std::vector<cv::Rect> &m_faces,
                                std::vector<cv::Rect> &m_eyes,
                                std::vector<cv::Rect> &m_noses,
                                std::vector<cv::Rect> &m_mouths,
                                float prob_threshold, float nms_threshold) {
    //detect ======================================================================
    cv::Mat rgb;
    if (format == 1) {
        cv::Mat src(h1 + h1 / 2, w1, CV_8UC1, data);
        cv::cvtColor(src, rgb, cv::COLOR_YUV2RGB_I420);
        src.release();
    } else if (format == 2 || format == 3) {
        cv::Mat src(h1 + h1 / 2, w1, CV_8UC2, data);
        cv::cvtColor(src, rgb, cv::COLOR_YUV2RGB_NV21);
        src.release();
    } else if (format == 4) {
        cv::Mat src(h1 + h1, w1, CV_8UC3, data);
        cv::cvtColor(src, rgb, cv::COLOR_RGBA2BGR);
        src.release();
    } else {
        return 0;
    }
    cv::resize(rgb, rgb, cv::Size(w1 / DETECT_SCALE, h1 / DETECT_SCALE));
    m_faces.clear();
    m_eyes.clear();
    m_noses.clear();
    m_mouths.clear();
    std::vector<FaceObject> faces;
    int width = rgb.cols;
    int height = rgb.rows;
    // insightface/detection/scrfd/configs/scrfd/scrfd_500m.py
    const int target_size = 640;
    // pad to multiple of 32
    int w = width;
    int h = height;
    float scale = 1.f;
    if (w > h) {
        scale = (float) target_size / w;
        w = target_size;
        h = h * scale;
    } else {
        scale = (float) target_size / h;
        h = target_size;
        w = w * scale;
    }
    ncnn::Mat in = ncnn::Mat::from_pixels_resize(rgb.data, ncnn::Mat::PIXEL_RGB,
                                                 width, height, w, h);
    // pad to target_size rectangle
    int wPad = (w + 31) / 32 * 32 - w;
    int hPad = (h + 31) / 32 * 32 - h;
    ncnn::Mat in_pad;
    ncnn::copy_make_border(in, in_pad, hPad / 2, hPad - hPad / 2, wPad / 2, wPad - wPad / 2,
                           ncnn::BORDER_CONSTANT, 0.f);
    const float meanVals[3] = {127.5f, 127.5f, 127.5f};
    const float normVals[3] = {1 / 128.f, 1 / 128.f, 1 / 128.f};
    in_pad.substract_mean_normalize(meanVals, normVals);
    ncnn::Extractor ex = m_DetectNet.create_extractor();
    ex.input("input.1", in_pad);
    std::vector<FaceObject> faceproposals;
    // stride 8
    {
        ncnn::Mat score_blob, bbox_blob, kps_blob;
        ex.extract("score_8", score_blob);
        ex.extract("bbox_8", bbox_blob);
        if (m_Kps) ex.extract("kps_8", kps_blob);
        const int base_size = 16;
        const int feat_stride = 8;
        ncnn::Mat ratios(1);
        ratios[0] = 1.f;
        ncnn::Mat scales(2);
        scales[0] = 1.f;
        scales[1] = 2.f;
        ncnn::Mat anchors = generate_anchors(base_size, ratios, scales);
        std::vector<FaceObject> faces32;
        generate_proposals(anchors, feat_stride, score_blob, bbox_blob, kps_blob, prob_threshold,
                           faces32);
        faceproposals.insert(faceproposals.end(), faces32.begin(), faces32.end());
    }
    // stride 16
    {
        ncnn::Mat score_blob, bbox_blob, kps_blob;
        ex.extract("score_16", score_blob);
        ex.extract("bbox_16", bbox_blob);
        if (m_Kps) ex.extract("kps_16", kps_blob);
        const int base_size = 64;
        const int feat_stride = 16;
        ncnn::Mat ratios(1);
        ratios[0] = 1.f;
        ncnn::Mat scales(2);
        scales[0] = 1.f;
        scales[1] = 2.f;
        ncnn::Mat anchors = generate_anchors(base_size, ratios, scales);
        std::vector<FaceObject> faces16;
        generate_proposals(anchors, feat_stride, score_blob, bbox_blob, kps_blob, prob_threshold,
                           faces16);
        faceproposals.insert(faceproposals.end(), faces16.begin(), faces16.end());
    }
    // stride 32
    {
        ncnn::Mat score_blob, bbox_blob, kps_blob;
        ex.extract("score_32", score_blob);
        ex.extract("bbox_32", bbox_blob);
        if (m_Kps) ex.extract("kps_32", kps_blob);
        const int base_size = 256;
        const int feat_stride = 32;
        ncnn::Mat ratios(1);
        ratios[0] = 1.f;
        ncnn::Mat scales(2);
        scales[0] = 1.f;
        scales[1] = 2.f;
        ncnn::Mat anchors = generate_anchors(base_size, ratios, scales);
        std::vector<FaceObject> faces8;
        generate_proposals(anchors, feat_stride, score_blob, bbox_blob, kps_blob, prob_threshold,
                           faces8);
        faceproposals.insert(faceproposals.end(), faces8.begin(), faces8.end());
    }
    // sort all proposals by score from highest to lowest
    qsort_descent_inplace(faceproposals);
    // apply nms with nms_threshold
    std::vector<int> picked;
    nms_sorted_bboxes(faceproposals, picked, nms_threshold);
    int face_count = picked.size();
    faces.resize(face_count);
    int i = 0;
    for (; i < face_count; i++) {
        faces[i] = faceproposals[picked[i]];
        // adjust offset to original unpadded
        float x0 = (faces[i].rect.x - (wPad / 2)) / scale;
        float y0 = (faces[i].rect.y - (hPad / 2)) / scale;
        float x1 = (faces[i].rect.x + faces[i].rect.width - (wPad / 2)) / scale;
        float y1 = (faces[i].rect.y + faces[i].rect.height - (hPad / 2)) / scale;
        x0 = std::max(std::min(x0, (float) width - 1), 0.f);
        y0 = std::max(std::min(y0, (float) height - 1), 0.f);
        x1 = std::max(std::min(x1, (float) width - 1), 0.f);
        y1 = std::max(std::min(y1, (float) height - 1), 0.f);
        faces[i].rect.x = x0;
        faces[i].rect.y = y0;
        faces[i].rect.width = x1 - x0;
        faces[i].rect.height = y1 - y0;
        if (m_Kps) {
            float x0 = (faces[i].landmark[0].x - (wPad / 2)) / scale;
            float y0 = (faces[i].landmark[0].y - (hPad / 2)) / scale;
            float x1 = (faces[i].landmark[1].x - (wPad / 2)) / scale;
            float y1 = (faces[i].landmark[1].y - (hPad / 2)) / scale;
            float x2 = (faces[i].landmark[2].x - (wPad / 2)) / scale;
            float y2 = (faces[i].landmark[2].y - (hPad / 2)) / scale;
            float x3 = (faces[i].landmark[3].x - (wPad / 2)) / scale;
            float y3 = (faces[i].landmark[3].y - (hPad / 2)) / scale;
            float x4 = (faces[i].landmark[4].x - (wPad / 2)) / scale;
            float y4 = (faces[i].landmark[4].y - (hPad / 2)) / scale;
            faces[i].landmark[0].x = std::max(std::min(x0, (float) width - 1), 0.f);
            faces[i].landmark[0].y = std::max(std::min(y0, (float) height - 1), 0.f);
            faces[i].landmark[1].x = std::max(std::min(x1, (float) width - 1), 0.f);
            faces[i].landmark[1].y = std::max(std::min(y1, (float) height - 1), 0.f);
            faces[i].landmark[2].x = std::max(std::min(x2, (float) width - 1), 0.f);
            faces[i].landmark[2].y = std::max(std::min(y2, (float) height - 1), 0.f);
            faces[i].landmark[3].x = std::max(std::min(x3, (float) width - 1), 0.f);
            faces[i].landmark[3].y = std::max(std::min(y3, (float) height - 1), 0.f);
            faces[i].landmark[4].x = std::max(std::min(x4, (float) width - 1), 0.f);
            faces[i].landmark[4].y = std::max(std::min(y4, (float) height - 1), 0.f);
        }
    }
    //draw/segmentation/landmark ====================================================
    LOGCATE("detection faces = %d", face_count);
    const unsigned char face_part_colors[8][3] = {{0,   0,   255},
                                                  {255, 85,  0},
                                                  {255, 170, 0},
                                                  {255, 0,   85},
                                                  {255, 0,   170},
                                                  {0,   255, 0},
                                                  {170, 255, 255},
                                                  {255, 255, 255}};
    i = 0;
    for (; i < faces.size(); i++) {
        FaceObject &obj = faces[i];
        //face segmentation
        /*cv::Mat mask = cv::Mat::zeros(256, 256, CV_8UC1);
        cv::Rect box;
        seg(rgb, obj, mask, box);
        cv::Mat maskResize;
        cv::resize(mask, maskResize, cv::Size(box.width, box.height), 0, 0, cv::INTER_NEAREST);
        for (size_t h = 0; h < maskResize.rows; h++) {
            cv::Vec3b *pRgb = rgb(box).ptr<cv::Vec3b>(h);
            for (size_t w = 0; w < maskResize.cols; w++) {
                int index = maskResize.at<uchar>(h, w);
                if (index == 0)
                    continue;
                pRgb[w] = cv::Vec3b(face_part_colors[index][2] * 0.3 + pRgb[w][2] * 0.7,
                                    face_part_colors[index][1] * 0.3 + pRgb[w][1] * 0.7,
                                    face_part_colors[index][0] * 0.3 + pRgb[w][0] * 0.7);
            }
        }*/
        //face mesh landmark
        std::vector<cv::Point2f> pts;
        onMesh(rgb, obj, pts);
        LOGCATE("landmark pts = %d", pts.size());
        i = 0;
        for (; i < pts.size(); i++) {
            cv::Rect face(obj.rect.x * DETECT_SCALE, obj.rect.y * DETECT_SCALE,
                          obj.rect.width * DETECT_SCALE, obj.rect.height * DETECT_SCALE);
            m_faces.push_back(face);
            cv::Rect eyeL(pts[0].x * DETECT_SCALE, pts[0].y * DETECT_SCALE, 10.0f, 10.0f);
            cv::Rect eyeR(pts[1].x * DETECT_SCALE, pts[1].y * DETECT_SCALE, 10.0f, 10.0f);
            m_eyes.push_back(eyeL);
            m_eyes.push_back(eyeR);
            LOGCATE("landmark ?= %d  %d", (int) pts[0].x, (int) pts[0].y);
            return 0;
        }
    }
    return 0;
}

void FaceNCNNDetection::onMesh(cv::Mat &rgb, FaceObject &obj,
                               std::vector<cv::Point2f> &landmarks) {
    int pad = obj.rect.height;
    cv::Rect box;
    box.x = (obj.rect.x + obj.rect.width / 2) - pad / 2;
    box.y = obj.rect.y;
    box.width = obj.rect.height;
    box.height = obj.rect.height;
    box.x = std::max(0.f, (float) box.x);
    box.y = std::max(0.f, (float) box.y);
    box.width = box.x + box.width < rgb.cols ? box.width : rgb.cols - box.x - 1;
    box.height = box.y + box.height < rgb.rows ? box.height : rgb.rows - box.y - 1;
    cv::Mat faceRoiImage = rgb(box).clone();
    ncnn::Extractor ex_face = m_MeshNet.create_extractor();
    ncnn::Mat ncnn_in = ncnn::Mat::from_pixels_resize(faceRoiImage.data, ncnn::Mat::PIXEL_RGB,
                                                      faceRoiImage.cols, faceRoiImage.rows,
                                                      192, 192);
    const float means[3] = {127.5f, 127.5f, 127.5f};
    const float norms[3] = {1 / 127.5f, 1 / 127.5f, 1 / 127.5f};
    ncnn_in.substract_mean_normalize(means, norms);
    ex_face.input("input.1", ncnn_in);
    ncnn::Mat ncnn_out;
    ex_face.extract("482", ncnn_out);
    float *scoredata = (float *) ncnn_out.data;
    int i = 0;
    for (; i < 468; i++) {
        cv::Point2f pt;
        pt.x = scoredata[i * 3] * box.width / 192 + box.x;
        pt.y = scoredata[i * 3 + 1] * box.width / 192 + box.y;
        landmarks.push_back(pt);
    }
}
}

//void FaceNCNNDetection::segmentation(cv::Mat &rgb, const FaceObject &obj,cv::Mat &mask,cv::Rect &box)
//{
//    int pad = obj.rect.height;
//    box.x = (obj.rect.x+obj.rect.width/2)-pad/2-20;
//    box.y = obj.rect.y-80;
//    box.width = obj.rect.height+40;
//    box.height = obj.rect.height+80;
//
//    box.x = std::max(0.f,(float)box.x);`1
//    box.y = std::max(0.f,(float)box.y);
//    box.width = box.x+box.width<rgb.cols?box.width:rgb.cols-box.x-1;
//    box.height = box.y+box.height<rgb.rows?box.height:rgb.rows-box.y-1;
//
//    cv::Mat faceRoiImage = rgb(box).clone();
//    ncnn::Extractor ex_face = faceseg.create_extractor();
//    ncnn::Mat ncnn_in = ncnn::Mat::from_pixels_resize(faceRoiImage.data,ncnn::Mat::PIXEL_RGB, faceRoiImage.cols, faceRoiImage.rows,256,256);
//
//    ncnn_in.substract_mean_normalize(meanVals, normVals);
//    ex_face.input("input",ncnn_in);
//    ncnn::Mat ncnn_out;
//    ex_face.extract("output",ncnn_out);
//    float *scoredata = (float*)ncnn_out.data;
//
//    unsigned char * maskIndex = mask.data;
//    int h = mask.rows;
//    int w = mask.cols;
//    for (int i = 0; i < h; i++)
//    {
//        for (int j = 0; j < w; j++)
//        {
//            int maxk = 0;
//            float tmp = scoredata[0 * w * h + i * w + j];
//            for (int k = 0; k < 8; k++)
//            {
//                if (tmp < scoredata[k * w * h + i * w + j])
//                {
//                    tmp = scoredata[k * w * h + i * w + j];
//                    maxk = k;
//                }
//            }
//            maskIndex[i * w + j] = maxk;
//        }
//    }
//    //cv::resize(mask,mask,faceRoiImage.size(),0,0,cv::INTER_NEAREST);
//
//}
//
