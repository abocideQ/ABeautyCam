#ifndef FFMPEGTEST_SIMPLEPLAYER_H
#define FFMPEGTEST_SIMPLEPLAYER_H

#include "VideoDecode.h"
#include "AudioDecode.h"

class VdPlayer {
public:
    void onSource(char *source);

    void onSeekTo(int percent);

    void onPlay();

    void onPause();

    void onStop();

    void onRelease();

    void onSurfaceCreated();

    void onSurfaceChanged(int w, int h);

    void onDrawFrame();

    static VdPlayer *instance();

protected:
    VideoRender *m_VideoRender = nullptr;

    VideoDecode *m_VideoDecode = nullptr;

    AudioRender *m_AudioRender = nullptr;

    AudioDecode *m_AudioDecode = nullptr;

private:
    static VdPlayer *m_Player;

};


#endif //FFMPEGTEST_SIMPLEPLAYER_H
