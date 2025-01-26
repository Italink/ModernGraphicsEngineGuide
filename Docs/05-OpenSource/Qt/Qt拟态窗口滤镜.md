---
comments: true
---

# Qt拟态窗口滤镜

- Github仓库：https://github.com/Italink/QNeumorphism

相信很多小伙伴都见到过一些拟态的界面效果，就像下面这样：

![img](Resources/8f7a30c7d7c35a32e18d0f85d58f00a2764139fa.png@1192w.webp)

拟态界面看起来即简洁又美观，很多前端小伙伴借助CSS3.0可以很轻松的实现这样的效果，但是Qt的QSS样式是基于CSS2.0的，并没有 **box-shadow** 属性，无法通过样式表来设置这样的效果，可笔者看着这个拟态效果心里又痒痒。

在一段时间的研究后，终于找到了解决方案—— **QGraphicsEffect**

这是一个使用拟态滤镜的Demo，其中核心文件只有三个：

- **QNeumorphism.h**
- **QNeumorphism.cpp**
- **QPixmapFilter.h**

该Demo参照 **https://neumorphism.io** ，实现了对 **QPushButton** 调整拟态滤镜参数的显示面板，下面是一些效果样例：

![img](Resources/b20b640de7f2fa1ef6f8dd04dcc73949a3a3f59c.png@604w_698h.webp)



![img](Resources/7a14c0abf7741a33643d12512002f96071f861c0.png@602w_702h.webp)
