---
comments: true
---

# Filtre de fenêtre néomorphique pour Qt

- Dépôt Github : https://github.com/Italink/QNeumorphism

Beaucoup d'entre vous ont sans doute déjà vu des effets d'interface néomorphiques, comme celui ci-dessous :

![img](../../../../05-OpenSource/Qt/Resources/8f7a30c7d7c35a32e18d0f85d58f00a2764139fa.png@1192w.webp)

Les interfaces néomorphiques sont à la fois simples et esthétiques. De nombreux développeurs front-end peuvent facilement réaliser ce type d'effet avec CSS3.0, mais le langage de style QSS de Qt est basé sur CSS2.0 et ne possède pas la propriété **box-shadow**, ce qui rend impossible la configuration de cet effet via les feuilles de style. Cependant, cet effet néomorphique m'a intrigué.

Après quelques recherches, j'ai enfin trouvé une solution : **QGraphicsEffect**

Ceci est un démonstrateur utilisant le filtre néomorphique, dont les fichiers principaux ne sont que trois :

- **QNeumorphism.h**
- **QNeumorphism.cpp**
- **QPixmapFilter.h**

Ce démonstrateur s'inspire de **https://neumorphism.io** et implémente un panneau d'affichage permettant de régler les paramètres du filtre néomorphique pour **QPushButton**. Voici quelques exemples d'effets :

![img](../../../../05-OpenSource/Qt/Resources/b20b640de7f2fa1ef6f8dd04dcc73949a3a3f59c.png@604w_698h.webp)



![img](../../../../05-OpenSource/Qt/Resources/7a14c0abf7741a33643d12512002f96071f861c0.png@602w_702h.webp)