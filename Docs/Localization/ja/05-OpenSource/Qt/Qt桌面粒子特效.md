---
comments: true
---

# Qtデスクトップパーティクルエフェクト

- Githubリポジトリ：https://github.com/Italink/DesktopParticlesEffect

最近『哪吒』を見て、非常にクールなパーティクルエフェクトがあることに気づきました：

![img](../../../../05-OpenSource/Qt/Resources/7cee99b178e3913b3ffe1f74a322ca91.gif)

突然自分で実装してみようと思い、これが最終的な効果です：

![img](../../../../05-OpenSource/Qt/Resources/6f459e67df921a7cf9b8862cb8dd54d7.gif)

## 使用ツール

少なくとも1つのGUIツールを習得する必要があります。ここではQt5+QtCreatorを使用しています。

## 習得が必要な内容

- 描画メカニズム
- マルチスレッド（リアルタイム画面更新）
- Windowsウィンドウプロパティの変更（ウィンドウの透明化、ボーダーの非表示、マウスの透過）

## アニメーションメカニズム

- 点：アニメーションには多くの漂う点（実際には非常に小さな円）が存在し、これらの点は異なる速度と方向で移動します。点が画面外に出るのを防ぐため、衝突メカニズムを追加する必要があります。点が画面に衝突した場合、移動方向を変更する必要があります（光の反射メカニズムに従う）。また、ここでは衝突時にランダムに点に速度を与えています。

- 線：上記の点の集合を検出し、2つの点の距離が特定の範囲に達した場合に線を描画します。また、線の透明度は線の長さに関連付けられています。

- 三角形：線を描画した後、点の集合を走査して、この点が三角形を構成できるかどうかを確認します。可能であれば描画を行い、透明度を関連付けます。
- マウス移動：1つの点を設定してマウスの位置を追跡することで、マウスの位置からも線と三角形を描画できるようにします。マウスの履歴軌跡を記録し、マウスの移動速度が特定の値に達した場合、ランダムに点の集合の1つの点を履歴軌跡上に配置します。

## 設定

![img](../../../../05-OpenSource/Qt/Resources/c8d801c45bcd904e7ac65427701242a2.png)

## 描画コード

``` c++
void Widget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);
    QPen pen;
    pen.setWidthF(1.2);
    painter.setPen(pen);
    QColor c(Config::lineColor);
    c.lighter();
    QPolygon p;
    for(unsigned int i=0;i<points.size();++i){      //重複除去に注意
        for(unsigned int j=i+1;j<points.size();++j){
            if(points[i].getDistance(points[j])<Config::maxLen_of_line){
                c.setAlpha(Config::maxLen_of_line-points[i].getDistance(points[j]));
                pen.setBrush(c);
                painter.setPen(pen);
                painter.drawLine(points[i].getX(),points[i].getY(),points[j].getX(),points[j].getY());      //線を描画
                c.setAlpha((Config::maxLen_of_line-points[i].getDistance(points[j]))/3);
                painter.setBrush(c);
                for(int k=j+1;k<points.size();++k){
                    if(points[k].getDistance(points[i])<Config::maxLen_of_line&&points[k].getDistance(points[j])<Config::maxLen_of_line){
                        p.setPoints(3,int(points[i].getX()),int(points[i].getY()),int(points[j].getX()),int(points[j].getY()),int(points[k].getX()),int(points[k].getY()));
                        painter.drawPolygon(p);                                 //三角形を描画
                    }
                }
            }
 
        }
        painter.setBrush(QColor(252,251,243));
        painter.setPen(Qt::NoPen);
        if(i)           //点を描画、最初の点はマウス用
            painter.drawEllipse(points[i].getRect());
    }
}
```

## 点の実行コード

``` c++
void Widget::run()
{
    collisionDetection();
 
    points[0].setX(QCursor::pos().x());
    points[0].setY(QCursor::pos().y());
    for(int i=1;i<points.size();++i)
    {
        points[i].run();
    }
    int x=(lastPos-QCursor::pos()).x(),
        y=(lastPos-QCursor::pos()).y();
    if(sqrt(x*x+y*y)>Config::len_of_link)
    {
        int index=1+qrand()%(points.size()-1);
        points[index].setX(lastPos.x());
        points[index].setY(lastPos.y());
    }
    lastPos=QCursor::pos();
    update();
}
```