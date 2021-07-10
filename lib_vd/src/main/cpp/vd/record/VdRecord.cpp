#include "VdRecord.h"

extern "C" {

void VdRecord::onConfig(char *urlOut, int width, int height, long vBitRate, int fps) {
    m_MediaRecord = new MediaRecord();
    MediaConfig *config = new MediaConfig();
    config->width = width;
    config->height = height;
    config->vBitRate = vBitRate;
    config->fps = fps;
    config->aSampleRate = 44100;
    config->aChannel = AV_CH_LAYOUT_STEREO;
    config->aFormat = AV_SAMPLE_FMT_S16;
    m_MediaRecord->onSource(urlOut, *config);
}

void VdRecord::onStart() {
    m_MediaRecord->onPrepare();
}
void VdRecord::onStop() {
    m_MediaRecord->onRelease();
}

void VdRecord::onBufferVideo(int format, int width, int height, uint8_t *data) {
    PixImage *image;
    image->format = format;
    image->width = width;
    image->height = height;
    image->plane[0] = data;
    if (format == IMAGE_FORMAT_YUV420P) {
        image->plane[1] = image->plane[0] + width * height;
        image->plane[2] = image->plane[1] + width * height / 4;
        image->pLineSize[0] = width;
        image->pLineSize[1] = width / 2;
        image->pLineSize[2] = width / 2;
    } else if (format == IMAGE_FORMAT_NV21 || format == IMAGE_FORMAT_NV12) {
        image->plane[1] = image->plane[0] + width * height;
        image->pLineSize[0] = width;
        image->pLineSize[1] = width;
    }
    VideoFrame *frame = new VideoFrame();
    frame->image = image;
    m_MediaRecord->onBufferVideo(frame);
}

void VdRecord::onBufferAudio(int size, uint8_t *data) {
    AudioFrame *frame = new AudioFrame(data, size, false);
    m_MediaRecord->onBufferAudio(frame);
}

VdRecord *VdRecord::m_Sample;
VdRecord *VdRecord::instance() {
    if (m_Sample == nullptr) {
        m_Sample = new VdRecord();
    }
    return m_Sample;
}
}
