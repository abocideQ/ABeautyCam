#include "GaussBlur.h"

extern "C" {

void GaussBlur::onFilter() {
    m_Thread = new std::thread(loopStart, this);
}

void GaussBlur::loopStart(GaussBlur *gauss) {
    do {
        gauss->fillEdge(0, 0, 0);
    } while (true);
}

void GaussBlur::fillEdge(int x, int y, int w) {
    int inx = x + i;
    if (inx < 0 || inx >= w) {
        return x - i;
    }
}
}





