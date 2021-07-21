#ifndef VDMAKE_GAUSSBLUR_H
#define VDMAKE_GAUSSBLUR_H

#import <thread>
#include <stdio.h>

class GaussBlur {
public:

    void onFilter();

protected:

    static void loopStart(GaussBlur *gauss);

    void fillEdge(int x, int y, int w);

private:

    float m_Radius = 3.0f;
    float m_Sigma = m_Radius / 3.0f;

    std::thread m_Thread;
    int m_Interrupt = 0;
};


#endif //VDMAKE_GAUSSBLUR_H
