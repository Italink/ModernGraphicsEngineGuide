---
comments: true
---

# Qtニューモーフィズムウィンドウフィルタ

- Githubリポジトリ：https://github.com/Italink/QNeumorphism

多くの方がニューモーフィズムのインターフェイス効果を見たことがあると思います。下記のようなものです：

![img](../../../../05-OpenSource/Qt/Resources/8f7a30c7d7c35a32e18d0f85d58f00a2764139fa.png@1192w.webp)

ニューモーフィズムインターフェイスはシンプルで美しく見えます。多くのフロントエンド開発者はCSS3.0を使って簡単にこの効果を実現できますが、QtのQSSスタイルはCSS2.0に基づいており、**box-shadow**プロパティがないため、スタイルシートでこの効果を設定することはできません。しかし筆者はこのニューモーフィズム効果を見て気になって仕方がありませんでした。

しばらく研究した後、ついに解決策を見つけました——**QGraphicsEffect**

これはニューモーフィズムフィルタを使用したDemoで、コアファイルはたった3つです：

- **QNeumorphism.h**
- **QNeumorphism.cpp**
- **QPixmapFilter.h**

このDemoは**https://neumorphism.io**を参照して、**QPushButton**のニューモーフィズムフィルタパラメータを調整する表示パネルを実現しています。下記はいくつかの効果サンプルです：

![img](../../../../05-OpenSource/Qt/Resources/b20b640de7f2fa1ef6f8dd04dcc73949a3a3f59c.png@604w_698h.webp)



![img](../../../../05-OpenSource/Qt/Resources/7a14c0abf7741a33643d12512002f96071f861c0.png@602w_702h.webp)