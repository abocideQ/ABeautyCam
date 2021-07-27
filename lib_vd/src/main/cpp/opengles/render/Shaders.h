#ifndef OPENGLESTEST_CAMERASHADER_H
#define OPENGLESTEST_CAMERASHADER_H

#define GL_SHADER_VERSION "#version 300 es \n"
#define GL_SHADER(x) #x
const char *ShaderVertex =
        GL_SHADER_VERSION
        GL_SHADER(
                layout(location = 0) in vec4 viPosition;
                layout(location = 1) in vec2 viTexCoord;
                out vec2 fiTexCoord;
                uniform mat4 vMatrix;
                void main() {
                    gl_Position = viPosition * vMatrix;
                    fiTexCoord = viTexCoord;
                }
        );
const char *ShaderFragment =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                layout(location = 0) out vec4 fragColor;
                uniform sampler2D s_TextureMap;
                void main() {
                    fragColor = texture(s_TextureMap, fiTexCoord);
                }
        );
const char *ShaderVertex_FBO =
        GL_SHADER_VERSION
        GL_SHADER(
                layout(location = 0) in vec4 viPosition;
                layout(location = 1) in vec2 viTexCoord;
                out vec2 fiTexCoord;
                uniform mat4 vMatrix;
                void main() {
                    gl_Position = viPosition * vMatrix;
                    fiTexCoord = viTexCoord;
                }
        );
const char *ShaderFragment_FBO_YUV420p =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D s_textureY;
                uniform sampler2D s_textureU;
                uniform sampler2D s_textureV;
                layout(location = 0) out vec4 fragColor;
                vec4 YUV420PtoRGB(vec2 texCoord) {
                    float y = 0.0f;
                    float u = 0.0f;
                    float v = 0.0f;
                    float r = 0.0f;
                    float g = 0.0f;
                    float b = 0.0f;
                    y = texture(s_textureY, texCoord).r;
                    u = texture(s_textureU, texCoord).r;
                    v = texture(s_textureV, texCoord).r;
                    u = u - 0.5;
                    v = v - 0.5;
                    r = y + 1.403 * v;
                    g = y - 0.344 * u - 0.714 * v;
                    b = y + 1.770 * u;
                    return vec4(r, g, b, 1.0f);
                }
                void main() {
                    fragColor = YUV420PtoRGB(fiTexCoord);
                }
        );
const char *ShaderFragment_FBO_NV21 =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D s_textureY;
                uniform sampler2D s_textureVU;
                layout(location = 0) out vec4 fragColor;
                vec4 NV21toRGB(vec2 texCoord) {
                    float y = 0.0f;
                    float u = 0.0f;
                    float v = 0.0f;
                    float r = 0.0f;
                    float g = 0.0f;
                    float b = 0.0f;
                    y = texture(s_textureY, texCoord).r;
                    u = texture(s_textureVU, texCoord).a;
                    v = texture(s_textureVU, texCoord).r;
                    u = u - 0.5;
                    v = v - 0.5;
                    r = y + 1.403 * v;
                    g = y - 0.344 * u - 0.714 * v;
                    b = y + 1.770 * u;
                    return vec4(r, g, b, 1.0f);
                }
                void main() {
                    fragColor = NV21toRGB(fiTexCoord);
                }
        );
const char *ShaderFragment_FBO_RGB =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D s_textureRGB;
                layout(location = 0) out vec4 fragColor;
                void main() {
                    fragColor = texture(s_textureRGB, fiTexCoord);
                }
        );
const char *ShaderFragment_FBO_YUV420p_Face =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D s_textureY;
                uniform sampler2D s_textureU;
                uniform sampler2D s_textureV;
                layout(location = 0) out vec4 fragColor;
                //yuv2rgb
                vec4 YUV420PtoRGB(vec2 texCoord) {
                    float y = 0.0f;
                    float u = 0.0f;
                    float v = 0.0f;
                    float r = 0.0f;
                    float g = 0.0f;
                    float b = 0.0f;
                    y = texture(s_textureY, texCoord).r;
                    u = texture(s_textureU, texCoord).r;
                    v = texture(s_textureV, texCoord).r;
                    u = u - 0.5;
                    v = v - 0.5;
                    r = y + 1.403 * v;
                    g = y - 0.344 * u - 0.714 * v;
                    b = y + 1.770 * u;
                    return vec4(r, g, b, 1.0f);
                }
                uniform vec2 fPixelSize;
                //眼睛
                uniform vec2 fEyeLeft;
                uniform vec2 fEyeRight;
                uniform vec2 fNose;
                uniform vec2 fMouthL;
                uniform vec2 fMouthR;
                uniform float fEyeScale;
                uniform float fEyeRadius;
                vec2 eyeScale(vec2 texCoord, int eye) {
                    vec2 result = texCoord;
                    vec2 current = result * fPixelSize;
                    if (eye == 1) { //左眼
                        float distance = distance(current, fEyeLeft);
                        if (distance < fEyeRadius) {
                            float gamma = 1.0 - fEyeScale *
                                                pow(smoothstep(0.0, 1.0, distance / fEyeRadius) -
                                                    1.0,
                                                    2.0);
                            result = fEyeLeft + gamma * (current - fEyeLeft);
                            result = result / fPixelSize;
                        }
                    } else {
                        float distance = distance(current, fEyeRight);
                        if (distance < fEyeRadius) {
                            float gamma = 1.0 - fEyeScale *
                                                pow(smoothstep(0.0, 1.0, distance / fEyeRadius) -
                                                    1.0,
                                                    2.0);
                            result = fEyeRight + gamma * (current - fEyeRight);
                            result = result / fPixelSize;
                        }
                    }
                    return result;
                }
                void main() {
                    vec2 texture = fiTexCoord * fPixelSize;
                    if (distance(texture, vec2(fEyeLeft.x, fEyeLeft.y)) < 10.0f) {
                        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
                    } else if (distance(texture, vec2(fEyeRight.x, fEyeRight.y)) < 10.0f) {
                        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
                    } else {
//                        vec2 newCoord = eyeScale(fiTexCoord, 1);
//                        newCoord = eyeScale(newCoord, 2);
                        fragColor = YUV420PtoRGB(fiTexCoord);
                    }
                }
        );
#endif //OPENGLESTEST_CAMERASHADER_H
