//
// Created by 86193 on 2023/3/21.
//
#ifndef AVEDITOR_REPACK_H
#define AVEDITOR_REPACK_H

#include "common.h"

class repack {

public:
    int repack_fmt(const std::string &,
                  const std::string &);

private:
    AVFormatContext *m_inAVFormatContext;
    AVFormatContext *m_outAVFormatContext;
    AVPacket *m_packet;
    int m_stream_index = 0;
    int m_stream_mapping_size = 0;
    int *m_stream_mapping = NULL;

    void repack_free();
};

#endif //AVEDITOR_REPACK_H
