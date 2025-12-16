---
comments: true
---

# Qt 데스크톱 물 표면 물결 효과

- Github 저장소：https://github.com/Italink/DesktopWaveEffect.git

<p style="text-align:center"><iframe width="560" height="315" src="//player.bilibili.com/player.html?isOutside=true&aid=753422025&bvid=BV1yk4y1B72b&cid=197938359&p=1&autoplay=false" scrolling="no" border="0" frameborder="0" framespacing="0" allowfullscreen="true"></iframe></p>

![img](../../../../05-OpenSource/Qt/Resources/6c6942fc23e6d499a06203ccb8e73992.png)

![img](../../../../05-OpenSource/Qt/Resources/8289ad0b59540b647ca654aa30e74e5e.png)

## 구현 도구

- Qt+OpenGL

## 구현 기능
- 물결 시뮬레이션：물 표면 파형 생성 및 물 표면의 광학 굴절 계산 완료
- 창을 데스크톱에 임베드
- 최상위 창이 데스크톱을 가리는지 감시，가리면 물결 새로 고침 중지
- 전역 마우스 후크

## 원리
### 파형 생성

![img](../../../../05-OpenSource/Qt/Resources/3341e5ac2f69635732774e4931be900e.png)


물 표면 파형은 진동원 거리（dis）를 매개변수로 하는 사인파로 근사할 수 있으며，여기서 A는 최대 진폭，F는 주파수，dis는 현재 점（xy）과 진동원 사이의 거리입니다. 본질적으로 이는 이변 함수로，이 함수를 통해 다음과 같은 효과의 곡면을 구성할 수 있습니다：

![img](../../../../05-OpenSource/Qt/Resources/586c6e1955cabe0fc4ee063b8a512ff3.png)

여기서 반단면은 일반적인 사인 함수입니다：

![img](../../../../05-OpenSource/Qt/Resources/3c7dc1d9403c5a1a8e4f290f470508a1.png)

이렇게 물 표면 파형의 계산이 완료되었나요？ 분명 그렇지 않습니다. 물 표면의 파형은 단순한 사인파가 아닙니다（블로거도 세부 사항을 모릅니다），하지만 일부 물 표면 효과를 관찰하면 물결은 종종 작은 원형 고리 같은 파형만 표시되고 시간이 지남에 따라 외부로 확산하는 것을 발견할 수 있습니다.

그래서 우리는 무엇을 해야 할까요？

우리는 사인파의一小段을 강조 표시하고 다른 부분을 거의 0으로 억제해야 하며，표시되는 이 부분은 시간이 지남에 따라 외부로 확산해야 합니다. 최대 진폭을 변경하여 이러한 효과를 구현할 수 있습니다，즉 진폭을 상수가 아닌 함수로 만들어야 합니다.仔细想想，어떤 함수가 우리의 요구를 충족시킬 수 있을까요？

물론 그것입니다——만능의 가우스 함수

![img](../../../../05-OpenSource/Qt/Resources/03f43d477268d22156de71a9e4cb4852.png)

가우스 함수에서：

a는 최대 진폭(피크)을 나타냅니다
b는 대칭축을 나타냅니다
c는 종 모양의 너비와 관련됩니다
a=1，b=10，c=2일 때，다음과 같은 그래프를 얻을 수 있습니다

![img](../../../../05-OpenSource/Qt/Resources/467cf1d556f30a9694c0cb3a31b9cbf9.png)

이 가우스 함수가 있으면，이를 사인 함수의 최대 진폭으로 사용합니다（즉两者相乘）

![img](../../../../05-OpenSource/Qt/Resources/59231a71c5180e6267a135d411c7afe9.png)

이전 매개변수를 사용하면，다음을 얻게 됩니다：

![img](../../../../05-OpenSource/Qt/Resources/258d7e8ac6d5b6e82d8c349b0135e8f6.png)

즉 대략 다음과 같은 파형입니다

![img](../../../../05-OpenSource/Qt/Resources/ebd7858b760fbf032b0d6c093b153058.png)

확산을 하기 위해，가우스 함수의 대칭축을 시간（T）과 연관시켜 시간이推移함에 따라 대칭축도 비례하여 증가시켜야 합니다. 확산 속도를 제어하는 다른 변수 V를 추가할 수 있습니다

![img](../../../../05-OpenSource/Qt/Resources/967df372e35dc944ad44f0594aabf745.png)

또한 변수 W로 물결의 너비를 제어합니다.

综上，우리가 얻은 물결의 곡면 방정식은 다음과 같습니다：

![img](../../../../05-OpenSource/Qt/Resources/fe7251b6ee301493c7bea2181085d5b1.png)

여기서：

- A는 최대 진폭을 나타냅니다
- V는 확산 속도를 나타냅니다
- T는 시간을 나타냅니다
- W는 너비 계수를 나타냅니다
- F는 주파수를 나타냅니다

### 물 표면 굴절 계산

이 단계에서，이미지의某一 픽셀 점이 물 표면 굴절을 거친 후 실제로 출력해야 하는 색상을 계산합니다. OpenGL에서 이 부분 코드는 프래그먼트 셰이더에서 완료됩니다（따라서 이전 단계도 프래그먼트 셰이더에서 진행됩니다）.

#### 법선 계산
물결의 반단면을 통해 데모를 진행합니다，사람의 시선은 위에서 아래로 내려보며 모델을 대략 다음과 같이 관찰합니다（빛이 물→공기→사람의 눈으로 진행되지만 유도를 위해 빛을 사람의 눈에서发出하는 것으로 반대로 생각합니다）

![img](../../../../05-OpenSource/Qt/Resources/cc5877c23a13299b58a318ab40f6c8c0.png)

이미지의某一 픽셀 점이 물 표면 굴절을 거친 후 실제로 텍스처 이미지의 어느 위치에 해당하는지 계산해야 합니다

시선이 물 표면 굴절을 거친 후의 방향을 계산해야 하며，이 계산을 위해서는 시선이 물 표면의 해당 점（평면）에서의 법선을 얻어야 합니다. 법선을 통해 입사 방향→굴절 방향의 계산을 완료할 수 있습니다

물 표면을 함수를 통해 구성하기 때문에 이론적으로 함수를 편미분하여 두 개의 접선 벡터를 계산하고 이를 외적하여 점 평면의 법선을 얻을 수 있지만，함수는 실제로 매우 복잡하여 미분 후 함수가 매우 방대해지고 계산도 어렵기 때문에，점 평면의 법선을 계산하기 위해投机取巧的 방식을 사용합니다：

점 평면 근처의 두 개의 공선이 아닌 점의 높이를 구하여 두 개의 방향 벡터를 구성하고 이를 외적하여 법선을 계산합니다. 이렇게 얻은 데이터는 매우 정확하지는 않지만 충분히 사용할 수 있으며 효율도 낮지 않습니다.

#### 굴절 광선의 방향 계산
GLSL은 굴절을 계산하기 위한 refract（입사 벡터，법선，굴절 상대 계수）함수를 제공합니다

따라서 입사 벡터(0,0,-1)와 법선을 알면 굴절 벡터를 구하는 것은 매우 간단하며 함수를 호출하면 됩니다（물→공기의 굴절 계수는 4/3，약 1.33입니다）

#### 좌표 오프셋 계산

![img](../../../../05-OpenSource/Qt/Resources/8dc1957a675635afc54577394646095c.png)

굴절 벡터를 얻은 후，굴절 벡터의 z값이 실제 높이（물 표면 높이+파형 높이）와 같아지도록拉伸하면 물 표면 굴절 후 xy의 오프셋 값을 구할 수 있으며，원래 좌표+오프셋 좌표로 물 표면 굴절 후의 좌표를 얻을 수 있습니다.另外 텍스처 좌표의取值 범위는 [0,1]이므로 창 너비에 따라 좌표 표준화를 진행해야 합니다.

### 성공！
블로거가 직접 구현한 프래그먼트 셰이더 코드를 첨부합니다：

``` glsl
#version 330 core
out vec4 FragColor;
 
uniform sampler2D texture;
 
uniform vec3 data[50];          //data는(마우스 x，마우스 y，실행 시간)을 전달：다중 진동원 지원
uniform int data_size;          //현재 유효한 data 길이
uniform vec2 screen_size;   //화면 크기
uniform float frequency;    //주파수
uniform float amplitude;    //최대 진폭
uniform float wave_width;   //물결의 너비
uniform float depth;        //수평면과 배경 이미지 사이의 깊이
uniform float speed;
 
in vec2 TexCoord;
 
void main()
{
    float height;
    float upHeight;
    float rightHeight;
    for(int i=0;i<data_size;i++){
        float time = data[i].z;    //마우스 클릭 후의 시간
        float dis = distance(data[i].xy,gl_FragCoord.xy);  //마우스 위치와 현재 프래그먼트 위치 사이의 거리
 
        float amplit = amplitude*pow(2.74,-(dis-time*speed)*(dis-time*speed)/2/(wave_width*wave_width))*sin(dis*frequency);      //현재 프래그먼트의 진폭 계산：여기서는 가우스 함수를 이용하여 현재 시간에 표시되는 파형을 강조 표시
        height += amplit*sin(dis*frequency);              //현재 파형의 높이
 
        dis=distance(data[i].xy,gl_FragCoord.xy+vec2(0,1));
 
        upHeight += amplitude*pow(2.74,-(dis-time*speed)*(dis-time*speed)/2/(wave_width*wave_width))*sin(dis*frequency)*sin(dis*frequency);
 
        dis=distance(data[i].xy,gl_FragCoord.xy+vec2(1,0));
 
        rightHeight += amplitude*pow(2.74,-(dis-time*speed)*(dis-time*speed)/2/(wave_width*wave_width))*sin(dis*frequency)*sin(dis*frequency);
    }
 
 
    vec3 up = vec3(0,1,upHeight-height);
 
    vec3 right = vec3(1,0,rightHeight-height);
 
    vec3 normal = cross(up,right);
 
    vec3 view = vec3(0,0,-1);
 
    vec3 re = refract(view,normal,1.33);
 
    vec2 coordOffset = re.xy*((height+depth)/re.z)/screen_size;
 
    FragColor = texture2D(texture,TexCoord+coordOffset);
}
```