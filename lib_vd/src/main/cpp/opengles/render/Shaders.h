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
const char *ShaderFragment_FBO_NV212RGB =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D nv21Y;
                uniform sampler2D nv21VU;
                layout(location = 0) out vec4 fragColor;
                vec3 NV21toRGB(vec2 texCoord) {
                    float y = 0.0f;
                    float u = 0.0f;
                    float v = 0.0f;
                    float r = 0.0f;
                    float g = 0.0f;
                    float b = 0.0f;
                    y = texture(nv21Y, texCoord).r;
                    u = texture(nv21VU, texCoord).a;
                    v = texture(nv21VU, texCoord).r;
                    u = u - 0.5;
                    v = v - 0.5;
                    r = y + 1.403 * v;
                    g = y - 0.344 * u - 0.714 * v;
                    b = y + 1.770 * u;
                    return vec3(r, g, b);
                }
                void main() {
                    fragColor = vec4(NV21toRGB(fiTexCoord), 1.0f);
                }
        );
const char *ShaderFragment_FBO_GaussBlurAWay =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D textureRGB;
                layout(location = 0) out vec4 fragColor;
                //纹理大小
                uniform vec2 fPixelSize;
                vec3 gaussBlur(vec2 texCoord) {
                    vec3 rgbBlur = texture(textureRGB, texCoord).rgb;
                    {
                        //高斯模糊，采样点周围4(SHIFT_SIZE*2)个点均进行一次采样，再求颜色平均值作为当前点的颜色
                        // 高斯算子左右偏移值，当偏移值为2时，高斯算子为 5x5
                        const int SHIFT_SIZE = 11;
                        vec4 blurCoords[SHIFT_SIZE];
                        // 横向/竖向偏移步距
                        vec2 stepOffset = vec2(0.0f);
                        if (fPixelSize.x == 0.0f) {
                            stepOffset = vec2(0.0f, 1.0f / fPixelSize.y);
                        }
                        if (fPixelSize.y == 0.0f) {
                            stepOffset = vec2(1.0f / fPixelSize.x, 0.0f);
                        }
                        for (int i = 0; i < SHIFT_SIZE; i++) {
//                            blurCoords[i] = vec4(texCoord.x - float(i + 1) * stepOffset.x,
//                                                 texCoord.y,
//                                                 texCoord.x + float(i + 1) * stepOffset.y,
//                                                 texCoord.y);
                            blurCoords[i] = vec4(texCoord.xy - float(i + 1) * stepOffset,
                                                 texCoord.xy + float(i + 1) * stepOffset);
                            rgbBlur += texture(textureRGB, blurCoords[i].xy).rgb;
                            rgbBlur += texture(textureRGB, blurCoords[i].zw).rgb;
                        }
                        rgbBlur = rgbBlur * 1.0 / float(2 * SHIFT_SIZE + 1);
                    }
                    return rgbBlur;
                }
                void main() {
                    fragColor = vec4(gaussBlur(fiTexCoord), 1.0f);
                }
        );
const char *ShaderFragment_FBO_HighPassGauss =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D textureRGB;
                uniform sampler2D textureGaussRGB;
                layout(location = 0) out vec4 fragColor;
                //纹理大小
                uniform vec2 fPixelSize;
                vec3 highPass(vec2 texCoord) {
                    vec3 rgbSource = texture(textureRGB, texCoord).rgb;
                    vec3 rgbBlur = texture(textureGaussRGB, texCoord).rgb;
                    vec3 rgbPass = vec3(0.0f);
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
                    return rgbPass;
                }
                void main() {
                    fragColor = vec4(highPass(fiTexCoord), 1.0f);
                }
        );
const char *ShaderFragment_FBO_Beauty =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D textureSource;
                uniform sampler2D textureGauss;
                uniform sampler2D texturePassGauss;
                layout(location = 0) out vec4 fragColor;
                vec3 beauty(vec2 texCoord) {
                    vec3 rgbSource = texture(textureSource, texCoord).rgb;
                    vec3 rgbBlur = texture(textureGauss, texCoord).rgb;
                    vec3 rgbPassBlur = texture(texturePassGauss, texCoord).rgb;
                    vec3 rgb = vec3(0.0f);
                    {
                        //磨皮
                        float beauty = 0.8f;//磨皮指数
                        // 调节蓝色通道值
                        float bVal = clamp((min(rgbSource.b, rgbBlur.b) - 0.2) * 5.0, 0.0, 1.0);
                        // 找到高反差后RGB通道的最大值
                        float maxCColor = max(max(rgbPassBlur.r, rgbPassBlur.g), rgbPassBlur.b);
                        // 计算当前的强度
                        float intensity = (1.0 - maxCColor / (maxCColor + 0.2)) * bVal * beauty;
                        // 混合输出结果
                        rgb = mix(rgbSource.rgb, rgbBlur.rgb, intensity);
                    }
                    {
                        //亮度 0.0
                        float brightness = -0.06;
                        rgb = rgb + vec3(brightness);
                    }
                    {
                        //曝光 0.0
                        float exposure = 0.3f;
                        rgb = rgb * pow(2.0, exposure);
                    }
                    {
                        //对比度 1.0
                        float contrast = 0.9f;
                        rgb = ((rgb - vec3(0.5f)) * contrast + vec3(0.5f));
                    }
                    {
                        //饱和度 1.0 Luma算法求算灰度：Gray = R0.2125 + G0.7154 + B*0.0721
                        float saturation = 0.96f;
                        float luminance = dot(rgb, vec3(0.2125, 0.7154, 0.0721));
                        vec3 grey = vec3(luminance);
                        rgb = mix(grey, rgb, saturation);
                    }
                    return rgb;
                }
                void main() {
                    fragColor = vec4(beauty(fiTexCoord), 1.0f);
                }
        );
const char *ShaderFragment_FBO_Sharpen =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D textureRGB;
                layout(location = 0) out vec4 fragColor;
                uniform vec2 fPixelSize;
                vec3 sharpen(vec2 texCoord) {
                    vec3 rgb = texture(textureRGB, texCoord).rgb;
                    {
                        //锐化  卷积：1 1 1 1 9 1 1 1 1
                        float sharp = 0.8f;
                        rgb = rgb * (1.0f + 4.0f * sharp);
                        vec2 stepOffset = vec2(1.0f / fPixelSize.x, 1.0f / fPixelSize.y);
                        rgb -= texture(textureRGB, texCoord - vec2(stepOffset.x, 0.)).rgb * sharp;
                        rgb -= texture(textureRGB, texCoord + vec2(stepOffset.x, 0.)).rgb * sharp;
                        rgb -= texture(textureRGB, texCoord - vec2(0., stepOffset.y)).rgb * sharp;
                        rgb -= texture(textureRGB, texCoord + vec2(0., stepOffset.y)).rgb * sharp;
                    }
                    return rgb;
                }
                void main() {
                    fragColor = vec4(sharpen(fiTexCoord), 1.0f);
                }
        );
const char *ShaderFragment_FBO_Smooth =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D textureRGB;
                layout(location = 0) out vec4 fragColor;
                uniform vec2 fPixelSize;
                vec3 sharpen(vec2 texCoord) {
                    vec3 rgb = texture(textureRGB, texCoord).rgb;
                    {
                        //均值滤波
                        float step = 0.0001;
                        rgb += texture(textureRGB, vec2(texCoord.x - step, texCoord.y - step)).rgb;
                        rgb += texture(textureRGB, vec2(texCoord.x + step, texCoord.y - step)).rgb;
                        rgb += texture(textureRGB, vec2(texCoord.x + step, texCoord.y + step)).rgb;
                        rgb += texture(textureRGB, vec2(texCoord.x - step, texCoord.y + step)).rgb;
                        rgb = rgb / 5.0f;
                    }
                    {
                        //中值滤波
                    }
                    {
                        //双边滤波
                    }
                    return rgb;
                }
                void main() {
                    fragColor = vec4(sharpen(fiTexCoord), 1.0f);
                }
        );
const char *ShaderFragment_FBO_BigEye =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D textureRgb;
                layout(location = 0) out vec4 fragColor;
                //纹理大小
                uniform vec2 fPixelSize;
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
                    resultTex = resultTex * fPixelSize;
                    float distance = distance(resultTex, eyeTex);
                    if (distance < fEyeRadius) {
//                        resultTex = vec2(texCoord.x / 2.0f + 0.25f, texCoord.y / 2.0f + 0.25f);
                        float gamma = distance / fEyeRadius;
                        gamma = 1.0 - fEyeScale * (1.0 - pow(gamma, 2.0f));
                        gamma = clamp(gamma, 0.0, 1.0);
                        resultTex = eyeTex + (resultTex - eyeTex) * gamma;
                    }
                    return resultTex / fPixelSize;
                }
                void main() {
                    vec2 newCoord = eyeScale(fiTexCoord, fEyeLeft);
                    newCoord = eyeScale(newCoord, fEyeRight);
                    fragColor = texture(textureRgb, newCoord);
                }
        );
const char *ShaderFragment_FBO_Transition =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D textureSource;
                uniform sampler2D textureNext;
                layout(location = 0) out vec4 fragColor;
                uniform float progress;
                vec4 getFromColor(vec2 color) {
                    return texture(textureSource, color);
                }
                vec4 getToColor(vec2 color) {
                    return texture(textureNext, color);
                }
                vec4 transition(vec2 p) {
                    vec2 block = floor(p.xy / vec2(16));
                    vec2 uv_noise = block / vec2(64);
                    uv_noise += floor(vec2(progress) * vec2(1200.0, 3500.0)) / vec2(64);
                    vec2 dist = progress > 0.0 ? (fract(uv_noise) - 0.5) * 0.3 * (1.0 - progress)
                                               : vec2(0.0);
                    vec2 red = p + dist * 0.2;
                    vec2 green = p + dist * .3;
                    vec2 blue = p + dist * .5;
                    return vec4(mix(getFromColor(red), getToColor(red), progress).r,
                                mix(getFromColor(green), getToColor(green), progress).g,
                                mix(getFromColor(blue), getToColor(blue), progress).b, 1.0);
                }
                void main() {
                    fragColor = transition(fiTexCoord);
                }
        );
const char *ShaderFragment_FBO_YUV420p_Display =
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
const char *ShaderFragment_FBO_NV21_Display =
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
const char *ShaderFragment_FBO_RGB_Display =
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
/*const char *ShaderFragment_FBO_Beauty2 =
        GL_SHADER_VERSION
        GL_SHADER(
                precision highp float;
                in vec2 fiTexCoord;
                uniform sampler2D textureRGB;
                layout(location = 0) out vec4 fragColor;
                //纹理大小
                uniform vec2 fPixelSize;
                uniform vec2 shift;
                vec3 gaussBlur(vec2 texCoord) {
                    vec3 rgbBlur = texture(textureRGB, texCoord).rgb;
                    {
                        vec2 stepOffset = vec2(1.0f / fPixelSize.x, 1.0f / fPixelSize.y);
                        vec2 blurCoordinates[20];
                        blurCoordinates[0] = texCoord.xy + stepOffset * vec2(0.0, -10.0);
                        blurCoordinates[1] = texCoord.xy + stepOffset * vec2(0.0, 10.0);
                        blurCoordinates[2] = texCoord.xy + stepOffset * vec2(-10.0, 0.0);
                        blurCoordinates[3] = texCoord.xy + stepOffset * vec2(10.0, 0.0);
                        blurCoordinates[4] = texCoord.xy + stepOffset * vec2(5.0, -8.0);
                        blurCoordinates[5] = texCoord.xy + stepOffset * vec2(5.0, 8.0);
                        blurCoordinates[6] = texCoord.xy + stepOffset * vec2(-5.0, 8.0);
                        blurCoordinates[7] = texCoord.xy + stepOffset * vec2(-5.0, -8.0);
                        blurCoordinates[8] = texCoord.xy + stepOffset * vec2(8.0, -5.0);
                        blurCoordinates[9] = texCoord.xy + stepOffset * vec2(8.0, 5.0);
                        blurCoordinates[10] = texCoord.xy + stepOffset * vec2(-8.0, 5.0);
                        blurCoordinates[11] = texCoord.xy + stepOffset * vec2(-8.0, -5.0);
                        blurCoordinates[12] = texCoord.xy + stepOffset * vec2(0.0, -6.0);
                        blurCoordinates[13] = texCoord.xy + stepOffset * vec2(0.0, 6.0);
                        blurCoordinates[14] = texCoord.xy + stepOffset * vec2(6.0, 0.0);
                        blurCoordinates[15] = texCoord.xy + stepOffset * vec2(-6.0, 0.0);
                        blurCoordinates[16] = texCoord.xy + stepOffset * vec2(-4.0, -4.0);
                        blurCoordinates[17] = texCoord.xy + stepOffset * vec2(-4.0, 4.0);
                        blurCoordinates[18] = texCoord.xy + stepOffset * vec2(4.0, -4.0);
                        blurCoordinates[19] = texCoord.xy + stepOffset * vec2(4.0, 4.0);
                        float sampleColor = rgbBlur.g * 20.0;
                        sampleColor += texture(textureRGB, blurCoordinates[0]).g;
                        sampleColor += texture(textureRGB, blurCoordinates[1]).g;
                        sampleColor += texture(textureRGB, blurCoordinates[2]).g;
                        sampleColor += texture(textureRGB, blurCoordinates[3]).g;
                        sampleColor += texture(textureRGB, blurCoordinates[4]).g;
                        sampleColor += texture(textureRGB, blurCoordinates[5]).g;
                        sampleColor += texture(textureRGB, blurCoordinates[6]).g;
                        sampleColor += texture(textureRGB, blurCoordinates[7]).g;
                        sampleColor += texture(textureRGB, blurCoordinates[8]).g;
                        sampleColor += texture(textureRGB, blurCoordinates[9]).g;
                        sampleColor += texture(textureRGB, blurCoordinates[10]).g;
                        sampleColor += texture(textureRGB, blurCoordinates[11]).g;
                        sampleColor += texture(textureRGB, blurCoordinates[12]).g * 2.0;
                        sampleColor += texture(textureRGB, blurCoordinates[13]).g * 2.0;
                        sampleColor += texture(textureRGB, blurCoordinates[14]).g * 2.0;
                        sampleColor += texture(textureRGB, blurCoordinates[15]).g * 2.0;
                        sampleColor += texture(textureRGB, blurCoordinates[16]).g * 2.0;
                        sampleColor += texture(textureRGB, blurCoordinates[17]).g * 2.0;
                        sampleColor += texture(textureRGB, blurCoordinates[18]).g * 2.0;
                        sampleColor += texture(textureRGB, blurCoordinates[19]).g * 2.0;
                        sampleColor = sampleColor / 48.0;

                        float highPass = rgbBlur.g - sampleColor + 0.5;
                        for (int i = 0; i < 5; i++) {
                            if (highPass <= 0.5)
                                highPass = highPass * highPass * 2.0;
                            else
                                highPass = 1.0 - ((1.0 - highPass) * (1.0 - highPass) * 2.0);
                        }
                        vec3 W = vec3(0.299, 0.587, 0.114);
                        float luminance = dot(rgbBlur, W);
                        float alpha = 0.5;
                        vec3 smoothColor = rgbBlur + (rgbBlur - vec3(highPass)) * alpha * 0.1;
                        rgbBlur = vec3(mix(smoothColor.rgb, max(smoothColor, rgbBlur), alpha));
                    }
                    return rgbBlur;
                }
                void main() {
                    fragColor = vec4(gaussBlur(fiTexCoord), 1.0f);
                }
        );*/
#endif //OPENGLESTEST_CAMERASHADER_H
