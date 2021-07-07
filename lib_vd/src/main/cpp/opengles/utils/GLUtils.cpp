#include "GLUtils.h"

extern "C" {
GLuint GLUtils::glProgram(const char *vertex, const char *fragment) {
    GLuint program = 0;
    GLuint vertexShader = glShader(GL_VERTEX_SHADER, vertex);
    if (!vertexShader) return program;
    GLuint fragmentShader = glShader(GL_FRAGMENT_SHADER, fragment);
    if (!fragmentShader) return program;
    program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

        glDetachShader(program, vertexShader);
        glDeleteShader(vertexShader);
        vertexShader = 0;
        glDetachShader(program, fragmentShader);
        glDeleteShader(fragmentShader);
        fragmentShader = 0;
        if (linkStatus == GL_FALSE) {
            GLint length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            if (length) {
                char *info = (char *) malloc((size_t) length);
                if (info) {
                    glGetProgramInfoLog(program, length, NULL, info);
                    LOGCATE("GLUtils::glProgram Could not link program:\n%s\n", info);
                    free(info);
                }
                glDeleteProgram(program);
                program = 0;
            }
        }
    }
    return program;
}

void GLUtils::glProgramDel(GLuint program) {
    glUseProgram(0);
    glDeleteProgram(program);
}

GLuint GLUtils::glShader(GLenum type, const char *p) {
    GLuint shader = 0;
    shader = glCreateShader(type);
    if (shader) {
        glShaderSource(shader, 1, &p, NULL);
        glCompileShader(shader);
        GLint shaderStatus = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderStatus);
        if (shaderStatus == GL_FALSE) {
            GLint length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            if (length) {
                char *info = (char *) malloc((size_t) length);
                if (info) {
                    glGetShaderInfoLog(shader, length, NULL, info);
                    if (type == GL_VERTEX_SHADER) {
                        LOGCATE("GLUtils::glShader Could not compile shader Vertex:\n%s\n", info);
                    } else if (type == GL_FRAGMENT_SHADER) {
                        LOGCATE("GLUtils::glShader Could not compile shader Fragment:\n%s\n", info);
                    } else {
                        LOGCATE("GLUtils::glShader Could not compile shader ????:\n%s\n", info);
                    }
                    free(info);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}
}


