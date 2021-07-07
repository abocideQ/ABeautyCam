#ifndef NDKER_GLUTILS_H
#define NDKER_GLUTILS_H

#include <stdlib.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "../../utils/Log.h"

class GLUtils {
public:
    static GLuint glProgram(const char *vertex, const char *fragment);

    static void glProgramDel(GLuint program);

private:
    static GLuint glShader(GLenum type, const char *p);
};

#endif //NDKER_GLUTILS_H
