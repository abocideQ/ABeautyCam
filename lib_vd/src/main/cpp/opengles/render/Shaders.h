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
const char *ShaderFragment_FBO_NV21_Face =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D s_textureY;
                uniform sampler2D s_textureVU;
                layout(location = 0) out vec4 fragColor;
                //nv21rgb
                vec3 NV21toRGB(vec2 texCoord) {
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
                    return vec3(r, g, b);
                }
                //纹理大小
                uniform vec2 fPixelSize;
                //脸
                uniform vec2 fFacePoint;
                uniform vec2 fFaceSize;
                // 高斯算子左右偏移值，当偏移值为5时，高斯算子为 5 x 5
                const int SHIFT_SIZE = 2;
                out vec4 blurCoords[SHIFT_SIZE];
                vec4 faceBeauty(vec2 texCoord, vec2 facePoint, vec2 faceSize) {
                    vec2 texture = texCoord * fPixelSize;
                    if (texture.x < facePoint.x || texture.y < facePoint.y) {
                        return vec4(NV21toRGB(texCoord), 1.0f);
                    }
                    else if (texture.x > facePoint.x + faceSize.x ||
                               texture.y > facePoint.y + faceSize.y) {
                        return vec4(NV21toRGB(texCoord), 1.0f);
                    }
                    vec3 rgbSource = NV21toRGB(texCoord);
                    vec3 rgb = NV21toRGB(texCoord);
                    //高斯模糊
                    {
                        // 偏移步距
                        vec2 stepOffset = vec2(1.0f / fPixelSize.y, 1.0f / fPixelSize.x);
                        // 偏移坐标
                        for (int i = 0; i < SHIFT_SIZE; i++) {
                            blurCoords[i] = vec4(texCoord.xy - float(i + 1) * stepOffset,
                                                 texCoord.xy + float(i + 1) * stepOffset);
                            rgb += NV21toRGB(blurCoords[i].xy);
                            rgb += NV21toRGB(blurCoords[i].zw);
                        }
                        rgb = rgb * 1.0 / float(2 * SHIFT_SIZE + 1);
                    }
                    //高通滤波->高反差保留
                    {
                        // 高通滤波之后的颜色
                        vec3 highPass = rgbSource - rgb;
                        // 强光程度
                        float intensity = 24.0;
                        // 对应混合模式中的强光模式(color = 2.0 * color1 * color2)，对于高反差的颜色来说，color1 和color2 是同一个
                        rgb.r = clamp(2.0 * highPass.r * highPass.r * intensity, 0.0, 1.0);
                        rgb.g = clamp(2.0 * highPass.g * highPass.g * intensity, 0.0, 1.0);
                        rgb.b = clamp(2.0 * highPass.b * highPass.b * intensity, 0.0, 1.0);
                    }
                    return vec4(rgb, 1.0f);
                }
                //眼睛
                uniform vec2 fEyeLeft;
                uniform vec2 fEyeRight;
                uniform vec2 fNose;
                uniform vec2 fMouthL;
                uniform vec2 fMouthR;
                uniform float fEyeScale;
                uniform float fEyeRadius;
                vec2 eyeScale(vec2 texCoord, vec2 eyeTex) {
                    vec2 resultTex = texCoord;
                    vec2 pixelTex = resultTex * fPixelSize;
                    float distance = distance(pixelTex, eyeTex);
                    if (distance < fEyeRadius) {
                        float gamma = pow(smoothstep(0.0, 1.0, distance / fEyeRadius) - 1.0, 2.0);
                        gamma = fEyeScale * gamma;
                        gamma = 1.0 - gamma;
                        resultTex = eyeTex + gamma * (pixelTex - eyeTex);
                        resultTex = resultTex / fPixelSize;
                    }
                    return resultTex;
                }
                void main() {
//                    vec2 texture = fiTexCoord * fPixelSize;
//                    if (distance(texture, vec2(fEyeLeft.x, fEyeLeft.y)) < 6.0f) {
//                        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
//                    } else if (distance(texture, vec2(fEyeRight.x, fEyeRight.y)) < 6.0f) {
//                        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
//                    } else if (distance(texture, vec2(fNose.x, fNose.y)) < 6.0f) {
//                        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
//                    } else if (distance(texture, vec2(fMouthL.x, fMouthL.y)) < 6.0f) {
//                        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
//                    } else if (distance(texture, vec2(fMouthR.x, fMouthR.y)) < 6.0f) {
//                        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
//                    } else {
//                        fragColor = NV21toRGB(fiTexCoord);
//                    }
//                    fragColor = vec4(NV21toRGB(newCoord),1.0f);
                    vec2 newCoord = eyeScale(fiTexCoord, fEyeLeft);
                    newCoord = eyeScale(newCoord, fEyeRight);
                    fragColor = faceBeauty(newCoord, fFacePoint, fFaceSize);
                }
        );
#endif //OPENGLESTEST_CAMERASHADER_H
