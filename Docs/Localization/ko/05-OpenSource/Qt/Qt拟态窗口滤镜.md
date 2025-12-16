---
comments: true
---

# Qt 뉴모피즘 윈도우 필터

- Github 저장소: https://github.com/Italink/QNeumorphism

많은 친구들이 아래와 같은 뉴모피즘 인터페이스 효과를 본 적이 있을 것입니다:

![img](../../../../05-OpenSource/Qt/Resources/8f7a30c7d7c35a32e18d0f85d58f00a2764139fa.png@1192w.webp)

뉴모피즘 인터페이스는 간결하면서도美观해 보입니다. 많은 프론트엔드 개발자들은 CSS3.0을 이용해 이런 효과를 쉽게 구현할 수 있지만, Qt의 QSS 스타일은 CSS2.0을 기반으로 하므로 **box-shadow** 속성이 없어 스타일시트를 통해 이런 효과를 설정할 수 없습니다. 하지만 저는 이 뉴모피즘 효과를 보고 마음이痒痒했습니다.

한동안 연구한 끝에 마침 해결책을 찾았습니다 — **QGraphicsEffect**

이것은 뉴모피즘 필터를 사용하는 Demo로, 핵심 파일은 세 개뿐입니다:

- **QNeumorphism.h**
- **QNeumorphism.cpp**
- **QPixmapFilter.h**

이 Demo는 **https://neumorphism.io** 를 참고하여 **QPushButton** 에 대한 뉴모피즘 필터 매개변수를 조정하는 표시 패널을 구현했습니다. 아래는 몇 가지 효과 샘플입니다:

![img](../../../../05-OpenSource/Qt/Resources/b20b640de7f2fa1ef6f8dd04dcc73949a3a3f59c.png@604w_698h.webp)



![img](../../../../05-OpenSource/Qt/Resources/7a14c0abf7741a33643d12512002f96071f861c0.png@602w_702h.webp)