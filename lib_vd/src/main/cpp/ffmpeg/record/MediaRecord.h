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
#include <unistd.h>
#include <queue>
#include <thread>
#include <mutex>

struct MediaConfig {
    //video
    int width;
    int height;
    long vBitRate;
    int fps;
    //audio
    int aBitRate = 96000;
    int aSampleRate;
    int aChannel;
    int aFormat;
};

class AVOutputStream {
public:
    AVCodecContext *m_CodecCtx = nullptr;
    AVStream *m_Stream = nullptr;
    AVFrame *m_Frame = nullptr;
    AVFrame *m_TmpFrame = nullptr;
    SwsContext *m_SwsCtx = nullptr;
    SwrContext *m_SwrCtx = nullptr;
    volatile int64_t m_NextPts = 0;
    volatile int m_EncodeEnd = 0;
    int m_SamplesCount = 0;
};

class MediaRecord {
public:
    void onSource(char *urlOut, MediaConfig config);

    void onPrepare();

    void onBufferVideo(int format, int width, int height, uint8_t *data);

    void onBufferAudio(AudioFrame *input);

    void onRelease();

protected:

    int initStream(AVFormatContext *fmtCtx, AVOutputStream *stream);

    int initCodec(AVCodecID codecId, AVCodec *codec, AVOutputStream *stream);

    int codeVFrame(AVOutputStream *stream);

    int codeAFrame(AVOutputStream *stream);

    void release();

    AVFormatContext *m_AVFormatContext = nullptr;
    AVOutputFormat *m_AVOutputFormat = nullptr;
    AVCodec *m_AudioCodec = nullptr;
    AVCodec *m_VideoCodec = nullptr;
    AVOutputStream m_VideoStream;
    AVOutputStream m_AudioStream;
    //队列
    std::queue<VideoFrame *> m_VideoQueue;
    std::queue<AudioFrame *> m_AudioQueue;
    //线程
    std::thread *m_Thread = nullptr;
    mutable std::mutex m_Mutex;
    bool m_Interrupt = false;
private:
    char *m_UrlOut = nullptr;
    MediaConfig m_Config;

    static void onRunAsy(MediaRecord *p);
};

#endif //VDMAKE_MEDIARECORD_H
