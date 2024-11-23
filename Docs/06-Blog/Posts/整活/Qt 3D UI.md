---
comments: true
date: 2023-10-17
authors:
  - Italink
categories:
  - 整活
---

整了个花活0.0，在没改Qt源码的情况下，可以很轻松地将一个QWidget进行3D变换。
原理：

- 通过前后两个四边形区域算出中心投影变换矩阵

- 利用中心投影变换矩阵在QApplication中重新映射鼠标位置相关的事件

- 通过QRHI和GraphicsEffect的机制，结合变换矩阵，重新绘制图像

- 根据目标四边形的位置，自动伸缩绘制区域
    目前还有一些问题：

- 弹出窗口的位置有偏差

- 焦点事件也对不上

不过现在能做好多有意思的东西了~

<iframe src="https://vdn6.vzuu.com/SD/652a8c62-6ce8-11ee-88b7-d2e6d33fcc65-v8_f2_t1_A2to2a0O.mp4?pkey=AAUFPh49yrEREFjNGW-ETwUINkkyFvAo8WBfpUU-D6dLjf_7x0tH7JZLjN4hV7Wp0Nax4nrkt9zhPXgE9RHXRKx7&bu=1513c7c2&c=avc.8.0&expiration=1732382722&f=mp4&pu=3a8548f7&v=ks6&pp=ChMxNDAxNjIzODY1NzM5NTc5MzkyGGMiC2ZlZWRfY2hvaWNlMhMxMzY5MDA1NjA4NTk5OTA0MjU3PXu830Q%3D&pf=Web&pt=zhihu" scrolling="no" seamless align="middle" border="0" frameborder="no" framespacing="0" allowfullscreen="true"></iframe>

