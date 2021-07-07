#ifndef FFMPEGTEST_VIDEOCODEC_H
#define FFMPEGTEST_VIDEOCODEC_H

#include "BaseCodec.h"
#include "VideoRender.h"

class VideoCodec : public BaseCodec {

public:
    void setRender(VideoRender *render);

    void onInit(char *url);

    void onResume();

    void onPause();

    void onStop();

    void onRelease();

protected:
    //视频转换工具Context
    SwsContext *m_SwsContext = nullptr;
    //转换后的帧
    AVFrame *m_FrameScale = nullptr;
    //转换后的数据
    int m_BufferSize = 0;
    uint8_t *m_FrameScaleBuffer = nullptr;

    void codecHandler(AVFrame *frame);

private:
    VideoRender *m_Render = nullptr;
};


#endif //FFMPEGTEST_VIDEOCODEC_H
