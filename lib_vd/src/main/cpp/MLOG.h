//
// Created by 86193 on 2023/3/22.
//
#ifndef AVEDITOR_MLOG_H
#define AVEDITOR_MLOG_H

#include <android/log.h>

#define LOG_TAG "ANDROID_LOG"
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

#endif //AVEDITOR_MLOG_H
