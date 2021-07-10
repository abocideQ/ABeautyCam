#ifndef VDMAKE_PIXAUDIO_H
#define VDMAKE_PIXAUDIO_H

#include <malloc.h>
#include <memory.h>

class AudioFrame {
public:
    AudioFrame(uint8_t *data, int dataSize, bool hardCopy = true) {
        this->dataSize = dataSize;
        this->data = data;
        this->hardCopy = hardCopy;
        if (hardCopy) {
            this->data = static_cast<uint8_t *>(malloc(this->dataSize));
            memcpy(this->data, data, dataSize);
        }


    ~AudioFrame() {
        if (hardCopy && this->data)
            free(this->data);
        this->data = nullptr;
    }

    uint8_t *data = nullptr;
    int dataSize = 0;
    bool hardCopy = true;
};

#endif //VDMAKE_PIXAUDIO_H
