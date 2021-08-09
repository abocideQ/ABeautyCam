#### Android 视频录制/播放/滤镜/人脸/美颜(高反差保留后缺少一次高斯模糊)
#### Camera2 OpenGL OpenSL OpenCV FFmpeg
#### ?????????????奇怪的小知识?????????????
#### Camera高宽数据：
```
Camera2 预览数据"高宽"与实际相反 
利用"fbo(frameBufferObject)" -> 矩阵旋转(横竖翻转 + 镜像翻转) + 窗口高宽互换 -> 正常图像+数据

矩阵旋转
matrix = projection * view * model;

窗口高宽互换
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, height, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
glViewport(0, 0, height, width); 
```
#### 视频常用编码格式：
```
JPEG
MPEG
H.263
MPEG-4
H.264/AVC
H.265/HEVC
```
#### 图像处理，卷积核的一些用法(大小必须为奇数：3x3/5x5 , 5x5的核 半径则为2)：
```
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
```

#### 高斯模糊 (正态分布的权重)
```
矩阵半径：R
标准差σ：Sigma（卷积矩阵半径的1/3为宜）
权重矩阵 = 二维高斯(x,y) -> 权重矩阵每个点之和为1保证亮度 -> 每个点进行(2R+1)*(2R+1)次运算
实际应用 = 一维高斯(x) +  一维高斯(y) -> 权重矩阵每个点之和为1保证亮度 -> 每个点进行(2R+1)*2次运算
高斯模糊 = 权重矩阵 x Source Pixel
```
高斯函数(正态分布的密度函数), 一维：

![Image text](https://github.com/ABCDQ123/VdMake/blob/main/lib_vd/image/gauss1d_origin.png)

μ是x的均值，σ是x的方差。计算平均值时，中心点为原点，所以μ=0。

![Image text](https://github.com/ABCDQ123/VdMake/blob/main/lib_vd/image/gauss1d.png)

根据一维高斯函数，可以推导得到二维高斯函数：

![Image text](https://github.com/ABCDQ123/VdMake/blob/main/lib_vd/image/gauss2d.png)

一维高斯函数 sqrt：平方根 exp：e的x次幂
```
double xxxClass::onGauss1D(int x) {
    double A = 1 / (Sigma * sqrt(2 * 3.1415926));
    double B = -1.0 * (x * x) / (2 * Sigma * Sigma);
    return A * exp(B);
}
```
```
https://gl-transitions.com/
https://blog.csdn.net/zfgrinm/article/details/79291693?spm=1001.2014.3001.5501
```