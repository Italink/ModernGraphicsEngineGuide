---
comments: true
---

# Qt 데스크톱 파티클 효과

- Github 저장소: https://github.com/Italink/DesktopParticlesEffect

최근 '哪吒（Ne Zha）'를 보면서 매우 멋진 파티클 효과를 발견했습니다:

![img](../../../../05-OpenSource/Qt/Resources/7cee99b178e3913b3ffe1f74a322ca91.gif)

갑자기 직접 구현해보고 싶었고, 이것이 최종 결과물입니다:

![img](../../../../05-OpenSource/Qt/Resources/6f459e67df921a7cf9b8862cb8dd54d7.gif)

## 사용 도구

최소한 하나의 GUI 도구를掌握해야 합니다: 여기서는 Qt5+QtCreator를 사용했습니다.

## 필요한 지식

- 그리기 메커니즘
- 멀티스레딩（실시간 화면 업데이트）
- Windows 창 속성 수정（창 투명화, 테두리 숨기기, 마우스穿透）

## 애니메이션 메커니즘

- 점: 애니메이션에는 많은 흩어지는 점（실제로는 매우 작은 원）이 있으며, 이 점들은 서로 다른 속도와 방향으로 움직입니다. 점이 화면 밖으로 나가는 것을 방지하기 위해 충돌 메커니즘을 추가해야 합니다. 점이 화면에 닿을 때 운동 방향을 변경（빛의 반사 메커니즘 준수）해야 하며, 여기서는 충돌 시 점에 랜덤한 속도를 부여합니다.

- 선: 위의 점 집합을 검사하여 두 점의 거리가 특정 범위에 도달하면 선을 그립니다. 선의 투명도는 선의 길이와 관련됩니다.

- 삼각형: 선을 그린 후 점 집합을 순회하여 해당 점이 삼각형을 구성할 수 있는지 확인합니다. 가능하면 그리며 투명도와 연관시킵니다.
- 마우스 이동: 마우스 위치를 추적하는 점을 설정하여 마우스 위치에서도 선과 삼각형을 그릴 수 있게 합니다. 마우스의历史轨迹를 기록하고, 마우스 이동速度가 특정 값에 도달하면 점 집합 중 하나의 점을历史轨迹上에 랜덤으로 배치합니다.

## 구성

![img](../../../../05-OpenSource/Qt/Resources/c8d801c45bcd904e7ac65427701242a2.png)

## 그리기 코드

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
    for(unsigned int i=0;i<points.size();++i){      //중복 제거에 주의
        for(unsigned int j=i+1;j<points.size();++j){
            if(points[i].getDistance(points[j])<Config::maxLen_of_line){
                c.setAlpha(Config::maxLen_of_line-points[i].getDistance(points[j]));
                pen.setBrush(c);
                painter.setPen(pen);
                painter.drawLine(points[i].getX(),points[i].getY(),points[j].getX(),points[j].getY());      //선 그리기
                c.setAlpha((Config::maxLen_of_line-points[i].getDistance(points[j]))/3);
                painter.setBrush(c);
                for(int k=j+1;k<points.size();++k){
                    if(points[k].getDistance(points[i])<Config::maxLen_of_line&&points[k].getDistance(points[j])<Config::maxLen_of_line){
                        p.setPoints(3,int(points[i].getX()),int(points[i].getY()),int(points[j].getX()),int(points[j].getY()),int(points[k].getX()),int(points[k].getY()));
                        painter.drawPolygon(p);                                 //삼각형 그리기
                    }
                }
            }
 
        }
        painter.setBrush(QColor(252,251,243));
        painter.setPen(Qt::NoPen);
        if(i)           //점 그리기, 첫 번째 점은 마우스로 사용
            painter.drawEllipse(points[i].getRect());
    }
}
```

## 점 실행 코드

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