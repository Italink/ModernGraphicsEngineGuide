---
comments: true
---

# Effet d'onde d'eau sur bureau avec Qt

- Dépôt Github : https://github.com/Italink/DesktopWaveEffect.git

<p style="text-align:center"><iframe width="560" height="315" src="//player.bilibili.com/player.html?isOutside=true&aid=753422025&bvid=BV1yk4y1B72b&cid=197938359&p=1&autoplay=false" scrolling="no" border="0" frameborder="0" framespacing="0" allowfullscreen="true"></iframe></p>

![img](../../../../05-OpenSource/Qt/Resources/6c6942fc23e6d499a06203ccb8e73992.png)

![img](../../../../05-OpenSource/Qt/Resources/8289ad0b59540b647ca654aa30e74e5e.png)

## Outils de réalisation

- Qt+OpenGL

## Fonctionnalités implémentées
- Simulation d'ondes d'eau : génération de formes d'onde sur l'eau, calcul de la réfraction lumineuse sur la surface de l'eau
- Intégration de la fenêtre dans le bureau
- Surveillance de l'occlusion du bureau par la fenêtre de premier plan : arrêt de la mise à jour des ondes d'eau si c'est le cas
- Hook souris global

## Principe
### Génération de formes d'onde

![img](../../../../05-OpenSource/Qt/Resources/3341e5ac2f69635732774e4931be900e.png)


La forme d'onde de la surface de l'eau peut être approximée par une onde sinusoïdale dont le paramètre est la distance (dis) à la source de vibration, où A est l'amplitude maximale, F la fréquence et dis la distance entre le point courant (xy) et la source de vibration.本质上这是一个二元函数,通过这样一个函数,我们能构造出如下效果的曲面：

![img](../../../../05-OpenSource/Qt/Resources/586c6e1955cabe0fc4ee063b8a512ff3.png)

La demi-section correspond à une fonction sinusoïdale classique :

![img](../../../../05-OpenSource/Qt/Resources/3c7dc1d9403c5a1a8e4f290f470508a1.png)

Est-ce que le calcul de la forme d'onde de la surface de l'eau est terminé ? Évidemment pas aussi simple, la forme d'onde de la surface de l'eau n'est pas seulement une onde sinusoïdale simple (le blogueur ne connaît pas non plus les détails), mais en observant certains effets d'eau, on peut constater que les ondes d'eau affichent souvent une petite section de forme de cercle et se propagent vers l'extérieur avec le temps.

Donc que devons-nous faire ?

Nous devons mettre en évidence une petite section de l'onde sinusoïdale et supprimer presque toutes les autres parties, et cette section affichée se propagera vers l'extérieur avec le temps. Nous pouvons réaliser cet effet en modifiant l'amplitude maximale, c'est-à-dire que nous devons faire de l'amplitude une fonction plutôt qu'une constante. Réfléchissons bien : quelle fonction peut répondre à nos besoins ?

Bien sûr, c'est lui — la fonction gaussienne omnipotente

![img](../../../../05-OpenSource/Qt/Resources/03f43d477268d22156de71a9e4cb4852.png)

Dans la fonction gaussienne :

a représente l'amplitude maximale (pic)
b représente l'axe de symétrie
c est lié à la largeur de la forme en cloche
Quand a=1, b=10 et c=2, vous obtenez ce graphique

![img](../../../../05-OpenSource/Qt/Resources/467cf1d556f30a9694c0cb3a31b9cbf9.png)

Avec cette fonction gaussienne, nous la utilisons comme amplitude maximale de la fonction sinusoïdale (c'est-à-dire leur produit)

![img](../../../../05-OpenSource/Qt/Resources/59231a71c5180e6267a135d411c7afe9.png)

En utilisant les paramètres précédents, nous obtiendrons :

![img](../../../../05-OpenSource/Qt/Resources/258d7e8ac6d5b6e82d8c349b0135e8f6.png)

C'est-à-dire une forme d'onde approximativement comme celle-ci

![img](../../../../05-OpenSource/Qt/Resources/ebd7858b760fbf032b0d6c093b153058.png)

Pour réaliser la propagation, nous devons lier l'axe de symétrie de la fonction gaussienne au temps (T), de sorte que l'axe de symétrie augmente avec le temps, c'est-à-dire qu'ils sont proportionnels. Nous pouvons ajouter une autre variable V pour contrôler la vitesse de propagation

![img](../../../../05-OpenSource/Qt/Resources/967df372e35dc944ad44f0594aabf745.png)

Ensuite, nous utilisons la variable W pour contrôler la largeur des ondes.

En résumé, l'équation de surface des ondes d'eau que nous obtenons est :

![img](../../../../05-OpenSource/Qt/Resources/fe7251b6ee301493c7bea2181085d5b1.png)

Où :

- A représente l'amplitude maximale
- V représente la vitesse de propagation
- T représente le temps
- W représente le coefficient de largeur
- F représente la fréquence

### Calcul de la réfraction sur la surface de l'eau

À cette étape, nous calculerons la couleur que devrait réellement afficher un pixel de l'image après réfraction sur la surface de l'eau. Dans OpenGL, ce code est exécuté dans le shader de fragment (donc l'étape précédente est également dans le shader de fragment).

#### Calcul de la normale
Nous utilisons une demi-section des ondes d'eau pour la démonstration. Le regard de l'homme est de haut en bas, et le modèle observé est approximativement comme ceci (bien que la lumière passe de l'eau à l'air puis à l'œil humain, nous inversons la direction pour la dérivation, considérant que la lumière émane de l'œil humain)

![img](../../../../05-OpenSource/Qt/Resources/cc5877c23a13299b58a318ab40f6c8c0.png)

Nous devons calculer quelle position de l'image de texture correspond réellement à un pixel de l'image après réfraction sur la surface de l'eau.

Nous avons besoin d'obtenir la direction de la lumière incidente après réfraction sur la surface de l'eau. Pour ce calcul, nous devons obtenir la normale du plan correspondant au point de la surface de l'eau, grâce à laquelle nous pouvons effectuer le calcul de la direction incidente vers la direction réfractée.

Puisque la surface de l'eau est construite par une fonction, théoriquement, nous pouvons calculer deux vecteurs tangentiels en dérivant la fonction, et leur produit vectoriel donnera la normale du plan du point. Cependant, la fonction est très complexe, et après dérivation, elle devient très volumineuse, ce qui rend le calcul difficile. Par conséquent, nous utilisons une méthode astucieuse pour calculer la normale du plan du point :

Nous calculons la hauteur de deux points non colinéaires à proximité du plan du point, formons deux vecteurs directionnels, et calculons la normale en effectuant leur produit vectoriel. Les données obtenues ne sont pas très précises, mais elles suffisent et l'efficacité n'est pas faible.

#### Calcul de la direction de la lumière réfractée
GLSL fournit une fonction refract (vecteur incident, normale, coefficient relatif de réfraction) pour calculer la réfraction.

Par conséquent, sachant le vecteur incident (0,0,-1) et la normale, il est très simple de calculer le vecteur réfracté en appelant la fonction (le coefficient de réfraction de l'eau à l'air est 4/3, environ 1.33).

#### Calcul du décalage des coordonnées

![img](../../../../05-OpenSource/Qt/Resources/8dc1957a675635afc54577394646095c.png)

Après avoir obtenu le vecteur réfracté, nous n'avons qu'à étirer le vecteur réfracté de sorte que la valeur z soit égale à la hauteur réelle (hauteur de la surface de l'eau + hauteur de la forme d'onde) pour obtenir le décalage xy après réfraction sur la surface de l'eau. La coordonnée d'origine + la coordonnée de décalage donne la coordonnée après réfraction sur la surface de l'eau. De plus, comme la plage des coordonnées de texture est [0,1], nous devons également normaliser les coordonnées en fonction de la largeur de la fenêtre.

### C'est fait !
Voici le code du shader de fragment implémenté par le blogueur :

``` glsl
#version 330 core
out vec4 FragColor;
 
uniform sampler2D texture;
 
uniform vec3 data[50];          //data传递(鼠标x，鼠标y，运行时间)：支持多振源
uniform int data_size;          //当前有效的data长度
uniform vec2 screen_size;   //屏幕尺寸
uniform float frequency;    //频率
uniform float amplitude;    //最大振幅
uniform float wave_width;   //波纹的宽度
uniform float depth;        //水平面距离背景图片的深度
uniform float speed;
 
in vec2 TexCoord;
 
void main()
{
    float height;
    float upHeight;
    float rightHeight;
    for(int i=0;i<data_size;i++){
        float time = data[i].z;    //鼠标点击后的时间
        float dis = distance(data[i].xy,gl_FragCoord.xy);  //鼠标位置到当前片段位置的距离
 
        float amplit = amplitude*pow(2.74,-(dis-time*speed)*(dis-time*speed)/2/(wave_width*wave_width))*sin(dis*frequency);      //计算当前片段的振幅：这里利用高斯函数突出显示当前时间所显示的波形
 
        height += amplit*sin(dis*frequency);              //当前波形的高度
 
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