#ifndef VDMAKE_FACENCNNDETECTION_H
#define VDMAKE_FACENCNNDETECTION_H

#include "net.h"
#include "cpu.h"
#include "opencv2/opencv.hpp"
#include "face_alignment.h"
#include "Log.h"

struct FaceObject {
    cv::Rect_<float> rect;
    cv::Point2f landmark[5];
    float prob;
};

class FaceNCNNDetection {
public:
    int onModel(char *folder, char *alignment, bool gpu = false);

    int onDetect(int format, int w1, int h1, uint8_t *data,
                 std::vector<cv::Rect> &m_faces,
                 std::vector<cv::Rect> &m_eyes,
                 std::vector<cv::Rect> &m_noses,
                 std::vector<cv::Rect> &m_mouths,
                 float prob_threshold = 0.4f,
                 float nms_threshold = 0.4f);

    void onMesh(cv::Mat &rgb, FaceObject &obj, std::vector<cv::Point2f> &landmarks);

private:
    //detection
    ncnn::Net m_DetectNet;
    bool m_Kps;
    //mesh
    ncnn::Net m_MeshNet;
    ncnn::Net m_SegNet;
    const float m_MeanVal[3] = {123.675f, 116.28f, 103.53f};
    const float m_NormVal[3] = {0.01712475f, 0.0175f, 0.01742919f};

    //faceAlignment
    cv::Ptr<seeta::FaceAlignment> m_Alignment;
    char *alignment_model = nullptr;
};

//utils methods
static inline float intersection_area(const FaceObject &a, const FaceObject &b) {
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

static void qsort_descent_inplace(std::vector<FaceObject> &faceobjects, int left, int right) {
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].prob;

    while (i <= j) {
        while (faceobjects[i].prob > p)
            i++;

        while (faceobjects[j].prob < p)
            j--;

        if (i <= j) {
            // swap
            std::swap(faceobjects[i], faceobjects[j]);

            i++;
            j--;
        }
    }

//     #pragma omp parallel sections
    {
//         #pragma omp section
        {
            if (left < j) qsort_descent_inplace(faceobjects, left, j);
        }
//         #pragma omp section
        {
            if (i < right) qsort_descent_inplace(faceobjects, i, right);
        }
    }
}

static void qsort_descent_inplace(std::vector<FaceObject> &faceobjects) {
    if (faceobjects.empty())
        return;

    qsort_descent_inplace(faceobjects, 0, faceobjects.size() - 1);
}

static void nms_sorted_bboxes(const std::vector<FaceObject> &faceobjects, std::vector<int> &picked,
                              float nms_threshold) {
    picked.clear();

    const int n = faceobjects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++) {
        areas[i] = faceobjects[i].rect.area();
    }

    for (int i = 0; i < n; i++) {
        const FaceObject &a = faceobjects[i];

        int keep = 1;
        for (int j = 0; j < (int) picked.size(); j++) {
            const FaceObject &b = faceobjects[picked[j]];

            // intersection over union
            float inter_area = intersection_area(a, b);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            //             float IoU = inter_area / union_area
            if (inter_area / union_area > nms_threshold)
                keep = 0;
        }

        if (keep)
            picked.push_back(i);
    }
}

// insightface/detection/scrfd/mmdet/core/anchor/anchor_generator.py gen_single_level_base_anchors()
static ncnn::Mat generate_anchors(int base_size, const ncnn::Mat &ratios, const ncnn::Mat &scales) {
    int num_ratio = ratios.w;
    int num_scale = scales.w;

    ncnn::Mat anchors;
    anchors.create(4, num_ratio * num_scale);

    const float cx = 0;
    const float cy = 0;

    for (int i = 0; i < num_ratio; i++) {
        float ar = ratios[i];

        int r_w = round(base_size / sqrt(ar));
        int r_h = round(r_w * ar); //round(base_size * sqrt(ar));

        for (int j = 0; j < num_scale; j++) {
            float scale = scales[j];

            float rs_w = r_w * scale;
            float rs_h = r_h * scale;

            float *anchor = anchors.row(i * num_scale + j);

            anchor[0] = cx - rs_w * 0.5f;
            anchor[1] = cy - rs_h * 0.5f;
            anchor[2] = cx + rs_w * 0.5f;
            anchor[3] = cy + rs_h * 0.5f;
        }
    }

    return anchors;
}

static void
generate_proposals(const ncnn::Mat &anchors, int feat_stride, const ncnn::Mat &score_blob,
                   const ncnn::Mat &bbox_blob, const ncnn::Mat &kps_blob, float prob_threshold,
                   std::vector<FaceObject> &faceobjects) {
    int w = score_blob.w;
    int h = score_blob.h;

    // generate face proposal from bbox deltas and shifted anchors
    const int num_anchors = anchors.h;

    for (int q = 0; q < num_anchors; q++) {
        const float *anchor = anchors.row(q);

        const ncnn::Mat score = score_blob.channel(q);
        const ncnn::Mat bbox = bbox_blob.channel_range(q * 4, 4);

        // shifted anchor
        float anchor_y = anchor[1];

        float anchor_w = anchor[2] - anchor[0];
        float anchor_h = anchor[3] - anchor[1];

        for (int i = 0; i < h; i++) {
            float anchor_x = anchor[0];

            for (int j = 0; j < w; j++) {
                int index = i * w + j;

                float prob = score[index];

                if (prob >= prob_threshold) {
                    // insightface/detection/scrfd/mmdet/models/dense_heads/scrfd_head.py _get_bboxes_single()
                    float dx = bbox.channel(0)[index] * feat_stride;
                    float dy = bbox.channel(1)[index] * feat_stride;
                    float dw = bbox.channel(2)[index] * feat_stride;
                    float dh = bbox.channel(3)[index] * feat_stride;

                    // insightface/detection/scrfd/mmdet/core/bbox/transforms.py distance2bbox()
                    float cx = anchor_x + anchor_w * 0.5f;
                    float cy = anchor_y + anchor_h * 0.5f;

                    float x0 = cx - dx;
                    float y0 = cy - dy;
                    float x1 = cx + dw;
                    float y1 = cy + dh;

                    FaceObject obj;
                    obj.rect.x = x0;
                    obj.rect.y = y0;
                    obj.rect.width = x1 - x0 + 1;
                    obj.rect.height = y1 - y0 + 1;
                    obj.prob = prob;

                    if (!kps_blob.empty()) {
                        const ncnn::Mat kps = kps_blob.channel_range(q * 10, 10);

                        obj.landmark[0].x = cx + kps.channel(0)[index] * feat_stride;
                        obj.landmark[0].y = cy + kps.channel(1)[index] * feat_stride;
                        obj.landmark[1].x = cx + kps.channel(2)[index] * feat_stride;
                        obj.landmark[1].y = cy + kps.channel(3)[index] * feat_stride;
                        obj.landmark[2].x = cx + kps.channel(4)[index] * feat_stride;
                        obj.landmark[2].y = cy + kps.channel(5)[index] * feat_stride;
                        obj.landmark[3].x = cx + kps.channel(6)[index] * feat_stride;
                        obj.landmark[3].y = cy + kps.channel(7)[index] * feat_stride;
                        obj.landmark[4].x = cx + kps.channel(8)[index] * feat_stride;
                        obj.landmark[4].y = cy + kps.channel(9)[index] * feat_stride;
                    }

                    faceobjects.push_back(obj);
                }

                anchor_x += feat_stride;
            }

            anchor_y += feat_stride;
        }
    }
}

#endif //VDMAKE_FACENCNNDETECTION_H
