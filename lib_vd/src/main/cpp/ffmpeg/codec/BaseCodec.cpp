#include "BaseCodec.h"

extern "C" {
std::mutex BaseCodec::m_Mutex;
void BaseCodec::onInit(char *url, AVMediaType mediaType) {
    onStop();
    m_Url = url;
    m_MediaType = mediaType;
    m_Thread = new std::thread(codecRunAsy, this);
}

void BaseCodec::onSeekTo(int percent) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_SeekPosition = percent;
}

void BaseCodec::onResume() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Status = STATE_RESUME;
}

void BaseCodec::onPause() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Status = STATE_PAUSE;
}

void BaseCodec::onStop() {
    if (m_Thread) {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Status = STATE_STOP;
        lock.unlock();
        m_Thread->join();
        delete m_Thread;
        m_Thread = nullptr;
    }
}

void BaseCodec::onRelease() {
    if (m_AVFormatContext != nullptr) {
        avformat_close_input(&m_AVFormatContext);
        avformat_free_context(m_AVFormatContext);
        m_AVFormatContext = nullptr;
    }
    if (m_AVCodecContext != nullptr) {
        avcodec_close(m_AVCodecContext);
        avcodec_free_context(&m_AVCodecContext);
        m_AVCodecContext = nullptr;
        m_AVCodec = nullptr;
    }
    if (m_Packet != nullptr) {
        av_packet_free(&m_Packet);
        m_Packet = nullptr;
    }
    if (m_Frame != nullptr) {
        av_frame_free(&m_Frame);
        m_Frame = nullptr;
    }
}

void BaseCodec::codecRunAsy(BaseCodec *ptr) {
    ptr->codecCreate();
    ptr->codecLoop();
}

void BaseCodec::codecCreate() {
    //FormatContext初始化
    if (m_AVFormatContext == nullptr) m_AVFormatContext = avformat_alloc_context();

    //open ???
    int open_state = avformat_open_input(&m_AVFormatContext, m_Url, NULL, NULL);
    if (open_state != 0) {
        LOGCATE("videoCodec avformat_open_input error %s", m_Url);
        char errorBuffer[128];
        if (av_strerror(open_state, errorBuffer, sizeof(errorBuffer)) == 0) {
            LOGCATE("videoCodec avformat_open_input error： %s", errorBuffer);
        }
        return;
    }

    //find 音视频流索引
    if (avformat_find_stream_info(m_AVFormatContext, NULL) < 0) {
        LOGCATE("videoCodec avformat_find_stream_info error");
        return;
    }
    int i = 0;
    for (; i < m_AVFormatContext->nb_streams; i++) {
        if (m_AVFormatContext->streams[i]->codecpar->codec_type == m_MediaType) {
            m_StreamIndex = i;
            break;
        }
    }
    if (m_StreamIndex == -1) {
        LOGCATE("videoCodec steamIndex find error");
        return;
    }

    //查找对应解码器
    AVCodecParameters *codecParameters = m_AVFormatContext->streams[m_StreamIndex]->codecpar;
    m_AVCodec = (AVCodec *) avcodec_find_decoder(codecParameters->codec_id);
    if (m_AVCodec == nullptr) {
        LOGCATE("videoCodec avcodec_find_decoder error");
        return;
    }
    //CodecContext初始化
    m_AVCodecContext = avcodec_alloc_context3(m_AVCodec);
    if (avcodec_parameters_to_context(m_AVCodecContext, codecParameters) != 0) {
        LOGCATE("videoCodec: avcodec_parameters_to_context error");
        return;
    }
    //打开解码器
//    AVDictionary *pAVDictionary = nullptr;
//    av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
//    av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
//    av_dict_set(&pAVDictionary, "max_delay", "30000000", 0);
//    av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);
    m_Result = avcodec_open2(m_AVCodecContext, m_AVCodec, nullptr);
    if (m_Result < 0) {
        LOGCATE("videoCodec: avcodec_open2 error");
        return;
    }
    //创建编码数据和解码数据的结构体
    m_Packet = av_packet_alloc();
    m_Frame = av_frame_alloc();
}

void BaseCodec::codecLoop() {
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Status = STATE_RESUME;
        m_StartTime = GetSysCurrentTime();
        m_Duration = m_AVFormatContext->duration / AV_TIME_BASE * 1000;
        lock.unlock();
    }
    for (;;) {
        while (m_Status == STATE_PAUSE) {
            continue;
        }
        if (m_Status == STATE_STOP) {
            return;
        }
        if (m_SeekPosition > 0) {
            int64_t seek_target = static_cast<int64_t>(m_SeekPosition * 1000000);//微秒
            int64_t seek_min = INT64_MIN;
            int64_t seek_max = INT64_MAX;
            int seek_ret = avformat_seek_file(m_AVFormatContext, -1, seek_min, seek_target,
                                              seek_max, 0);
            if (seek_ret >= 0) {
                m_SeekPosition = -1;
                avcodec_flush_buffers(m_AVCodecContext);
            } else {
                m_SeekPosition = 0;
                LOGCATE("videoCodec: avformat_seek_file error");
            }
        }
        while (av_read_frame(m_AVFormatContext, m_Packet) >= 0) {
            if (m_Packet->stream_index == m_StreamIndex) {
                if (avcodec_send_packet(m_AVCodecContext, m_Packet) == AVERROR_EOF) {
                    LOGCATE("videoCodec: avcodec_send_packet error");
                    goto __EXIT;
                }
                int frameCount = 0;
                while (avcodec_receive_frame(m_AVCodecContext, m_Frame) == 0) {
                    codecHandler(m_Frame);
                    synchronization();
                    frameCount++;
                }
                if (frameCount > 0) {
                    goto __EXIT;
                }
            }
            goto __EXIT;
        }
        __EXIT:
        av_packet_unref(m_Packet);
    }
}

void BaseCodec::synchronization() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    long curFrameTime = 0l;
    if (m_Frame->pkt_dts != AV_NOPTS_VALUE) {
        curFrameTime = m_Frame->pkt_dts;
    } else if (m_Frame->pts != AV_NOPTS_VALUE) {
        curFrameTime = m_Frame->pts;
    } else {
        curFrameTime = 0;
    }
    curFrameTime = (int64_t) (
            (curFrameTime * av_q2d(m_AVFormatContext->streams[m_StreamIndex]->time_base)) *
            1000);
    if (m_SeekPosition == -1) {
        m_StartTime = GetSysCurrentTime() - curFrameTime;
        m_SeekPosition = 0;
    }
    lock.unlock();
    long curSysTime = GetSysCurrentTime();
    long curPasTime = curSysTime - m_StartTime;//基于系统时钟计算从开始播放流逝的时间
    if (curFrameTime > curPasTime) {
        auto sleepTime = static_cast<unsigned int>(curFrameTime - curPasTime);//休眠时间ms
        sleepTime = sleepTime > 100 ? 100 : sleepTime; //限制休眠时间不能过长
        av_usleep(sleepTime * 1000);//时钟
    }
    long delay = curPasTime - curFrameTime;
}
}

