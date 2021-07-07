#ifndef FFMPEGTEST_BASECODEC_H
#define FFMPEGTEST_BASECODEC_H

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
};

#include "Log.h"
#include <thread>
#include <mutex>

enum DecoderState {
    STATE_UNKNOWN,
    STATE_RESUME,
    STATE_PAUSE,
    STATE_STOP
};

static long long GetSysCurrentTime() {
    struct timeval time;
    gettimeofday(&time, NULL);
    long long curTime = ((long long) (time.tv_sec)) * 1000 + time.tv_usec / 1000;
    return curTime;
}

class BaseCodec {
public:
    void onInit(char *url, AVMediaType mediaType);

    void onSeekTo(int percent);

    void onResume();

    void onPause();

    void onStop();

    void onRelease();

protected:
    //地址
    char *m_Url = nullptr;
    //媒体类型 AUDIO/VIDEO
    AVMediaType m_MediaType = AVMEDIA_TYPE_UNKNOWN;
    //打开结果
    int m_Result = -1;

    //解封装Context
    AVFormatContext *m_AVFormatContext = nullptr;
    //编码器
    AVCodec *m_AVCodec = nullptr;
    //编码器Context
    AVCodecContext *m_AVCodecContext = nullptr;
    //编码包
    AVPacket *m_Packet = nullptr;
    //帧数据
    AVFrame *m_Frame = nullptr;
    //音视频流索引
    int m_StreamIndex = 0;
    //音频播放开始时间
    long m_StartTime = 0l;
    //总时长 ms
    long m_Duration = 0l;
    //seekTo
    volatile float m_SeekPosition = 0;

    //线程
    std::thread *m_Thread = nullptr;
    volatile int m_Status = STATE_UNKNOWN;
    //互斥锁
    static std::mutex m_Mutex;
private:

    static void codecRunAsy(BaseCodec *ptr);

    void codecCreate();

    void codecLoop();

    void synchronization();

    virtual void codecHandler(AVFrame *frame) = 0;
};


#endif //FFMPEGTEST_BASECODEC_H
