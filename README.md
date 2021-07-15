视频常用编码格式
```
JPEG
MPEG
H.263
MPEG-4
H.264/AVC
H.265/HEVC
```
```
Android Camera 录制：
Camera提供的高宽 与 实际相反
利用fbo： 旋转(坐标系 + 镜像问题) + 互换高宽 得到正常数据
matrix = projection * view * model;
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, height, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
glViewport(0, 0, height, width);
```
人脸五官检测
https://blog.csdn.net/cfan927/article/details/72585587
https://github.com/dengjiaming/YUV2RGBA
