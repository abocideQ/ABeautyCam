#ifndef VDMAKE_MEDIARECORD_H
#define VDMAKE_MEDIARECORD_H

extern "C" {
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include "PixImage.h"
#include "PixAudio.h"
#include <queue>
#include <thread>
#include <mutex>


class MediaRecord {

protected:
    AVFormatContext *m_AVFormatContext;
    AVOutputFormat *m_AVOutputFormat = nullptr;
    AVCodec *m_AudioCodec = nullptr;
    AVCodec *m_VideoCodec = nullptr;
    //队列
    typedef PixImage VideoFrame;
    std::queue<VideoFrame *> m_VideoQueue;
    std::queue<AudioFrame *> m_AudioQueue;
    //线程
    std::thread *m_Thread = nullptr;
private:
    char *m_OutUrl = nullptr;
};

#endif //VDMAKE_MEDIARECORD_H
