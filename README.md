```
视频常用编码格式：
JPEG
MPEG
H.263
MPEG-4
H.264/AVC
H.265/HEVC
```
```
Android 视频录制
Camera2 + OpenGL + FFmpeg

Camera2提供的高宽 与 实际相反 , 利用fbo -> 旋转(坐标系 + 镜像问题) + 互换高宽 得到正常图像与数据

矩阵旋转
matrix = projection * view * model;

窗口高宽互换
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, height, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
glViewport(0, 0, height, width); 
```
```
图像处理，卷积核的一些用法(大小必须为奇数：3x3/5x5 , 5x5的核 半径则为2)：
空卷积核
0   0   0
0   1   0  x Source Pixel
0   0   0

锐化
-1  -1  -1
-1   9  -1  x Source Pixel
-1  -1  -1

浮雕
-1  -1   0
-1   0   1  x Source Pixel
 0   1   1

均值模糊
 0     0.2    0
0.2     0    0.2  x Source Pixel
 0     0.2    0

所谓模糊,可以理解成 "每一个像素" 都取周边像素的平均值, 模糊半径越大，图像就越模糊. 即：
Source Pixel: 1 1 1          1 1 1
              1 2 1 x ?? ->  1 1 1
              1 1 1          1 1 1
简单平均，显然不合理，因为图像都是连续的，越靠近的点关系越密切，越远离的点关系越疏远。
因此，加权平均更合理，距离越近的点权重越大，距离越远的点权重越小。

高斯模糊 (正态分布的权重)

```
