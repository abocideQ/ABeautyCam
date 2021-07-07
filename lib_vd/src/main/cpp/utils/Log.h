#ifndef NDKER_LOG_H
#define NDKER_LOG_H

#include<android/log.h>

#define LOG_TAG "ffmpeg_log"
#define LOGCATE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define GL_ERROR_CHECK(...) LOGCATE("GL_ERROR_CHECK %s error = %d, line = %d, ",  __FUNCTION__, glGetError(), __LINE__)
#endif //NDKER_LOG_H
