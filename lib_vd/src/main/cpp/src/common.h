//
// Created by 86193 on 2023/3/21.
//
#ifndef AVEDITOR_COMMON_H
#define AVEDITOR_COMMON_H

//platform
#include "MLOG.h"

//stl
#include <string>
#include <vector>
#include <map>

//ffmpeg
extern "C" {
#include "ffmpeg/include/libavformat/avformat.h"
#include "ffmpeg/include/libavcodec//avcodec.h"
#include "ffmpeg/include/libswscale/swscale.h"
#include "ffmpeg/include/libswresample/swresample.h"
#include "ffmpeg/include/libavutil/imgutils.h"
#include "ffmpeg/include/libavutil/timestamp.h"
#include "ffmpeg/include/libavutil/channel_layout.h"
#include "ffmpeg/include/libavutil/opt.h"
}


#endif //AVEDITOR_COMMON_H
