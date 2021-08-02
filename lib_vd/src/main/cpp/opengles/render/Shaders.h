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
                // 高斯算子左右偏移值，当偏移值为5时，高斯算子为 11 x 11
                const int SHIFT_SIZE = 5;
                out vec4 blurCoords[SHIFT_SIZE];
                vec4 faceBeauty(vec2 texCoord, vec2 facePoint, vec2 faceSize) {
                    vec2 texture = texCoord * fPixelSize;
                    if (texture.x < facePoint.x || texture.y < facePoint.y) {
                        return vec4(NV21toRGB(texCoord), 1.0f);
                    } else if (texture.x > facePoint.x + faceSize.x ||
                               texture.y > facePoint.y + faceSize.y) {
                        return vec4(NV21toRGB(texCoord), 1.0f);
                    }
                    vec3 rgbSource = NV21toRGB(texCoord);
                    vec3 rgbBlur = NV21toRGB(texCoord);
                    vec3 rgbPass = vec3(0.0f);
                    vec3 rgb = vec3(0.0f);
                    //高斯模糊
                    {
                        // 偏移步距
                        vec2 stepOffset = vec2(1.0f / fPixelSize.y, 1.0f / fPixelSize.x);
                        // 偏移坐标
                        for (int i = 0; i < SHIFT_SIZE; i++) {
                            blurCoords[i] = vec4(texCoord.xy - float(i + 1) * stepOffset,
                                                 texCoord.xy + float(i + 1) * stepOffset);
                            rgbBlur += NV21toRGB(blurCoords[i].xy);
                            rgbBlur += NV21toRGB(blurCoords[i].zw);
                        }
                        rgbBlur = rgbBlur * 1.0 / float(2 * SHIFT_SIZE + 1);
                    }
                    //高通滤波->高反差保留
                    {
                        // 高通滤波之后的颜色
                        rgbPass = rgbSource - rgbBlur;
                        // 强光程度
                        float intensity = 24.0;
                        // 对应混合模式中的强光模式(color = 2.0 * color1 * color2)，对于高反差的颜色来说，color1 和color2 是同一个
                        rgbPass.r = clamp(2.0 * rgbPass.r * rgbPass.r * intensity, 0.0, 1.0);
                        rgbPass.g = clamp(2.0 * rgbPass.g * rgbPass.g * intensity, 0.0, 1.0);
                        rgbPass.b = clamp(2.0 * rgbPass.b * rgbPass.b * intensity, 0.0, 1.0);
                    }
                    //磨皮
                    {
                        // 调节蓝色通道值
                        float bVal = clamp((min(rgbSource.b, rgbBlur.b) - 0.2) * 5.0, 0.0, 1.0);
                        // 找到高反差后RGB通道的最大值
                        float maxCColor = max(max(rgbPass.r, rgbPass.g), rgbPass.b);
                        // 计算当前的强度
                        float beauty = 0.8f;
                        float intensity = (1.0 - maxCColor / (maxCColor + 0.2)) * bVal * beauty;
                        // 混合输出结果
                        rgb = mix(rgbSource.rgb, rgbBlur.rgb, intensity);
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
