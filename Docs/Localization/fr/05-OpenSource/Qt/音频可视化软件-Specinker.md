---
comments: true
---

# Logiciel de visualisation audio — Specinker (déjà fermé source)

## **Préface**

Je crois que beaucoup d'entre vous aiment la musique, et moi aussi je suis un fan de musique fidèle, j'aime toujours rester seul sans rien faire et écouter de la musique tranquillement.

Vous avez peut-être déjà utilisé des logiciels de visualisation audio il y a longtemps, comme le logiciel de rendu post-production AE, WallpaperEngine, Rainmeter, Avee, etc. Il faut avouer que l'effet de spectre d'AE est absolument puissant, mais son temps de rendu a toujours été un problème très agaçant. Quant à WE et Rainmeter, ils se concentrent sur les logiciels de fond d'écran, et les matériaux de spectre sont extrêmement rares. Avee est un bon choix, si Spec ne répond pas à vos besoins, je vous recommande vivement de l'utiliser : il peut entrer de l'audio et encoder des vidéos, et ses effets sont également bons.

L'objectif initial de Spec était de permettre à l'auteur d'apprendre des connaissances liées à l'audio-vidéo et aux graphiques. Je n'imaginais pas que le développement irait aussi loin : la position du logiciel est passée d'un lecteur musical affichant un spectre au début, à un concepteur de spectre, puis à un moteur graphique de visualisation audio aujourd'hui. Je ose l'appeler un moteur graphique parce qu'il possède déjà beaucoup de fonctionnalités de moteur graphique.

Si vous êtes intéressé par l'auteur et le processus de développement de Spec, une brève explication est disponible à la fin.

## **Lien de téléchargement :**

Lien de téléchargement : [specinker5.1.exe - Lanzou Cloud](https://italink.lanzoui.com/iYOAZp16qib) (Si le logiciel plante, mettez à jour le pilote de votre carte graphique et ouvrez-le avec la carte graphique dédiée)

Ancien blog : [Ancien blog de Specinker_ItaLink-CSDN Blog](https://blog.csdn.net/qq_40946921/article/details/108539935)

Tutoriel d'utilisation : [Tutoriel d'utilisation de Spec_ItaLink-CSDN Blog_spec software](https://blog.csdn.net/qq_40946921/article/details/108528317)

Présentation PPT de la soutenance de fin d'études de licence :

<p style="text-align:center"><iframe width="560" height="315" src="//player.bilibili.com/player.html?bvid=BV1f44y1z7qi&cid=348421166&p=1&share_source=copy_web&autoplay=false" scrolling="no" border="0" frameborder="0" framespacing="0" allowfullscreen="true"></iframe></p>
## **Qu'est-ce que Specinker ?**

Specinker est un moteur graphique de visualisation audio que l'auteur a créé avec beaucoup de travail pendant un an. Il possède de nombreux effets graphiques, y compris des graphiques de spectre de base, des filtres spéciaux, un système de particules, des modèles 3D, etc. L'auteur a encapsulé ces graphiques et ouvert certains paramètres ajustables. Vous pouvez même écrire des scripts Lua pour ces paramètres pour réaliser des effets dynamiques (par exemple, rotation automatique, suivi de la souris, déplacement au rythme de la musique...).

## **Tutoriel vidéo**

<p style="text-align:center"><iframe width="560" height="315" src="//player.bilibili.com/player.html?isOutside=true&aid=335187087&bvid=BV1XA411c7vb&cid=398619724&p=1&autoplay=false" scrolling="no" border="0" frameborder="0" framespacing="0" allowfullscreen="true"></iframe></p>

## **Points forts des fonctionnalités de Specinker**

### **UI简洁高效 (UI simple et efficace)**

Le cadre global de l'interface graphique de Spec a connu cinq versions avec le changement de la position du logiciel. Voici des esquisses de conception partielles de quatre interfaces historiques : actuellement, Spec utilise une interface graphique plus simplifiée, à fenêtre unique et hautement contrôlable.

![img](../../../../05-OpenSource/Qt/Resources/f00ca0353c0d21e2fe7e5c7267fbc9f80a345c60.gif@1192w.webp)

L'auteur a réimplémenté des contrôles de réglage légers à haute performance :

![img](../../../../05-OpenSource/Qt/Resources/0aa4762c7dd6b7c322e1abe8610256e837a68845.png@1192w.webp)

### **Gestion des matériaux**

Cliquez sur le bouton 【+】 dans l'interface 【Ajustements】 pour ajouter un matériau. Après l'ajout, un ID par défaut sera attribué à ce matériau. Cliquez avec le bouton droit de la souris pour effectuer des opérations sur le matériau, et un simple clic ouvrira les contrôles de réglage de la position du matériau.

![img](../../../../05-OpenSource/Qt/Resources/32298a70405579a447f34f6fde3cc4759c4d0fc0.gif@1050w_1688h.webp)

**Groupement :** Maintenez la touche Ctrl pour sélectionner plusieurs carrés de matériaux. Après la sélection, cliquez avec le bouton droit de la souris pour ouvrir le menu et effectuer le groupement. Après le groupement, vous pouvez déplacer l'ensemble.
### **Gestion des propriétés**

Pour les propriétés de n'importe quel matériau, vous pouvez cliquer avec le bouton droit de la souris pour ouvrir le menu, vous pouvez continuer à copier 【Ctrl+C】 et coller 【Ctrl+V】, et utiliser l'annulation 【Ctrl+Z】 et le rétablissement 【Ctrl+Y】. Il faut noter que ces opérations s'appliquent à la propriété actuelle.

![img](../../../../05-OpenSource/Qt/Resources/31afc872a444280769c19f05e6e9d424ada951c0.png@654w_226h.webp)

En outre, vous pouvez ajouter la propriété 【à la valeur fréquente】, de sorte que cette propriété apparaîtra toujours dans la liste 【Valeurs fréquentes】 des propriétés du même nom. L'auteur en a ajouté quelques-unes, par exemple 【Rotation automatique】 pour 【Angle de rotation】.

Vous pouvez également gérer ces valeurs dans la gestion des valeurs fréquentes de l'interface 【Paramètres】 de Spec :

![img](../../../../05-OpenSource/Qt/Resources/e0a4224dcbfae75d271383958557975a40d98204.png@686w_1432h.webp)



![img](../../../../05-OpenSource/Qt/Resources/b5a27406834b4f16d3d4f838ac7738268130fbf8.png@1192w_894h.webp)

### **基础图形 (Graphiques de base)** 

Spec a implémenté quelques graphiques de base simples pour les utilisateurs. Vous pouvez combiner ces graphiques pour former de nouveaux effets, comme ceci :

![img](../../../../05-OpenSource/Qt/Resources/92d132fe52a2a5bffdfb154f7518e7b3a6508489.gif@1192w.webp)

### **实时滤镜 (Filtres en temps réel)**

Spec a implémenté de nombreux filtres en temps réel. L'utilisation de filtres peut rendre les graphiques très beaux, mais il faut noter que les filtres consomment beaucoup de ressources, donc veuillez essayer de réduire la zone du filtre.

![img](../../../../05-OpenSource/Qt/Resources/86a0bc30b9a99e09a92a1af176fbdff0d1edc16e.png@1192w.webp)

### **脚本引擎 (Moteur de script)**

Pour que une propriété d'un matériau change dynamiquement, par exemple, si vous voulez souvent qu'un graphique tourne automatiquement, Spec utilisait initialement une option 【Vitesse de rotation】 associée à 【Angle de rotation】 pour y parvenir. Cette méthode d'implémentation présente de nombreux inconvénients :

- 【Vitesse de rotation】 est susceptible de entrer en conflit avec 【Angle de rotation】 et n'est pas facile à synchroniser
- Il ne peut que réaliser une rotation automatique simple, sans possibilité d'extension (par exemple, il est impossible de faire tourner le graphique automatiquement au rythme de la musique)

La méthode actuellement utilisée par Spec est l'utilisation de scripts Lua. Vous pouvez écrire directement du code Lua pour que les propriétés des matériaux changent dynamiquement. Les propriétés pour lesquelles le code peut être utilisé ont une case à cocher devant elles, comme ceci :

![img](../../../../05-OpenSource/Qt/Resources/de19c58fe44d908c57dfa12907c26843aa60d85c.png@638w_426h.webp)

Il suffit de cocher la case, et la propriété utilisera le code pour le calcul. Par exemple, pour réaliser un effet de rotation automatique :

1. Cochez la case du code

![img](../../../../05-OpenSource/Qt/Resources/67a19f3cc9fb72ac161a3a2c545d56cb1cbde3c4.png@614w_362h.webp)

2. Cliquez sur le bouton d'édition du code pour ouvrir l'éditeur de code et écrire le code

![img](../../../../05-OpenSource/Qt/Resources/dcf714e3528bd8f70638501347b33cec686e1fa5.png@1192w_1192h.webp)

3. Cliquez sur Test pour exécuter, et vous verrez que le graphique commence à tourner automatiquement

![img](../../../../05-OpenSource/Qt/Resources/404ef14d9c2f032cc10f5f18066575799e4f3069.gif@1050w_1010h.webp)

4. Nous pouvons également publier nos propres paramètres. Cliquez avec le bouton droit sur la position et ajoutez une variable float1

![img](../../../../05-OpenSource/Qt/Resources/7bf1ce6e23aa54b337b0280ed5b82f61e6ab303e.png@752w_378h.webp)

5. Et nommez-la 【Vitesse de rotation】

![img](../../../../05-OpenSource/Qt/Resources/b85437b6691a1828986a692fdbbb0a387b60433f.png@548w_512h.webp)

6. Ensuite, remplacez 100 dans le code par 【Vitesse de rotation】

![img](../../../../05-OpenSource/Qt/Resources/7f4b8a608cbf37f24eb62d80a19a35f5b6969dc2.png@458w_70h.webp)

7. Ensuite, vous pouvez utiliser le paramètre 【Vitesse de rotation】 pour ajuster !

### **粒子系统 (Système de particules)**

Spec a implémenté un système de particules GPU programmable sur la base du moteur de script et d'OpenGL.

Le système de particules par défaut a ajouté du code pour la gravité et l'attraction, ainsi que le suivi de la souris. Ajoutez-lui un filtre de lueur, et vous trouverez que l'effet est en fait assez bon.

![img](../../../../05-OpenSource/Qt/Resources/64ec2c8d9b856388ad54b67f1a77969696163a61.png@1192w.webp)

Expliquons en détail comment utiliser le système de particules :

**Contrôler la forme des particules :**

La forme des particules est contrôlée par 【Type de primitive】 et 【Données de vertex】. Par défaut, le 【Type de primitive】 utilisé est le triangle fan, et le code Lua génère un polygone à n côtés :

![img](../../../../05-OpenSource/Qt/Resources/be91b0b5b86e5c912c52c4932ccaefacfc298158.png@640w_556h.webp)

Vous ne comprenez peut-être pas le code, mais il est essentiel de savoir comment Spec analyse le format des données de vertex. Vous devez construire un tableau Lua, où chaque trio de données représente respectivement xyz d'un vertex. Deux points à noter :

- Les coordonnées des vertex utilisent des coordonnées normalisées, c'est-à-dire que la plage de valeurs de xyz est [-1, 1]
- L'indice de départ du tableau Lua est 1

Par exemple, si vous voulez créer une forme de particule carrée, vous pouvez construire des données comme celles-ci dans le code :

![img](../../../../05-OpenSource/Qt/Resources/c61768052087375147804fc87b8f63f76440f18c.png@1118w_256h.webp)

Cliquez sur 【Test pour exécuter】 et changez 【Type de primitive】 en 【Bande de triangles】, et vous verrez que la forme de la particule devient un carré :

![img](../../../../05-OpenSource/Qt/Resources/3f6678638354efb657c16109983a2e49dbfa6ff5.png@734w_390h.webp)

**【Nombre de générations】 :** Indique combien de particules sont générées par seconde

**【Cycle de vie】 :** Indique combien de secondes chaque particule survit

**Explication de la programmabilité des particules :**

Les particules ne sont rien d'autre qu'une combinaison de données avec certains états (comme la vitesse, la taille, la position, la rotation, etc.). Le mouvement des particules consiste à modifier ces états de particules. Le système de particules de Spec utilise deux interfaces de code :

**【Générateur de particules】 :** Utilisé pour générer l'état initial des particules, qui peut être considéré comme l'état de la première frame (cet état ne s'affiche pas).

**【Processeur de mouvement】 :** Après la génération des particules, ce code est appelé à chaque frame pour traiter l'état des particules.

Il faut noter spécialement :

- Ici, l'implémentation n'est plus réalisée par le code Lua. Comme Spec utilise des particules GPU, implémentées par feedback de transformation, ce code s'exécute finalement sur le GPU. Par conséquent, le format de code utilisé ici est GLSL (langage de shader)
- Lisez attentivement le journal de la console dans le processeur de mouvement. Le mot-clé in indique une variable d'entrée. Pour différencier, l'auteur a utilisé des noms commençant par i, par exemple la variable 【iPosition】 représente 【position】 de la frame précédente. Il suffit d'affecter une valeur à 【position】 dans le code.

Grâce au code GLSL, vous pouvez définir n'importe quelle règle de mouvement de particules, par exemple, vous pouvez ajouter de la gravité, de la résistance, et même réaliser des effets de collision. Comme le contenu est abondant, l'auteur publiera un article détaillé ultérieurement.

## **À propos de l'auteur et de Specinker**

**L'auteur** est un petit frère qui aime se cacher dans un coin pour développer des technologies "noires". La première fois qu'il a接触é le spectre musical, c'était par hasard sur le fond d'écran d'un ami. C'était trop cool ! Vous avez peut-être déjà deviné quel logiciel c'était : oui, Rainmeter. Je suis immédiatement allé chercher des informations pour apprendre, mais j'ai découvert que la syntaxe de programmation de Rainmeter était complètement différente de celle de C/C++ que j'apprenais. C'était trop ancien. À ce moment-là, je me suis demandé : "Vaut-il la peine de passer du temps à apprendre cela ?" Évidemment, j'ai abandonné. J'ai aussi pensé à essayer de le faire moi-même, mais je n'ai pas su par où commencer.

Jusqu'à ce que l'auteur apprenne Qt plus tard et trouve un projet de spectromètre dans les exemples officiels. Je n'étais pas moins qu'excité à ce moment-là, mais les sept ou huit mille lignes de code étaient comme une montagne qui m'écrasait pour l'auteur de l'époque. Je ne connaissais pas le codage audio-vidéo, les pilotes multimédias, et même Qt, que j'avais appris depuis seulement quelques mois. Mais ne pouvais-je pas faire un projet même s'il y avait un projet à copier ?

J'ai donc commencé à éliminer le code inutile en fonction de la logique du code, comme si je désamorçais une bombe. Si le projet plantait, j'utilisais la sauvegarde précédente pour recommencer à supprimer. Après plus de deux semaines, j'ai finalement réduit le code core à seulement plus de mille lignes. Au cours de ce processus, j'ai également acquis une compréhension générale du flux de traitement audio. J'ai donc utilisé cette partie du code, combinée au mécanisme de dessin de Qt, pour terminer la première version de Specinker :

![img](../../../../05-OpenSource/Qt/Resources/119ede5368a2fe29eb4c7472a33659a1558412b9.png@1192w.webp)

En regardant en arrière maintenant, n'est-ce pas terrible ? Mais j'ai passé plus de deux mois sur ce petit truc. Cependant, j'ai reçu des encouragements de la part de mes camarades à l'époque, ce qui était très réconfortant. J'ai ensuite publié ce projet en ligne, et il s'est avéré qu'il y avait des gens qui l'utilisaient ! J'ai alors commencé à l'étendre comme un fou. J'ai transformé le lecteur de spectre original en un concepteur de spectre :

![img](../../../../05-OpenSource/Qt/Resources/099da36c1b6e0b86b9b7af032035e6779a0b0637.png@1192w.webp)

Mais au milieu du développement, j'ai soudainement réalisé que j'étais stupide :既然这个是制作好显示在桌面上，那我为什么不直接在桌面上进行设计，还弄个预览框，脱裤子放屁。

J'ai donc dû abandonner une partie du code et le réécrire. Ainsi, j'ai obtenu la version suivante (c'est-à-dire Dynamic Audio, qui est probablement la version la plus utilisée en ligne. Merci à tout le monde pour la promotion) :

![img](../../../../05-OpenSource/Qt/Resources/f93a7bd08ff6bdeb3cf3648810770282817d5b69.png@1192w.webp)

Cette version a apporté de grands changements à l'expérience utilisateur : elle était plus simple et plus facile à utiliser. Le volume de code était déjà important, dépassant les 10 000 lignes. À ce moment-là, j'étais très excité et j'ai dit à mon professeur : "Je trouvais le sélecteur de couleur de Qt trop moche, donc j'en ai fait un moi-même, et j'ai écrit plus de 2 000 lignes de code !" J'avais l'intention de continuer à étendre cette version, mais un problème est survenu : "Auteur, le spectre sur le bureau semble avoir un peu de retard, et les graphiques sont un peu laggy." J'ai alors expliqué avec patience : "Cet outil consomme beaucoup de ressources, un peu de lag est normal." Je pensais que c'était le cas jusqu'à ce que quelqu'un compare l'effet de Spec à celui de Wallpaper Engine. J'ai alors réalisé que j'avais sous-estimé C++ et surévalué moi-même. Que faire ? Je devais le modifier, sinon je serais un lâche. J'ai donc réécrit essentiellement le code core que j'avais obtenu des exemples officiels de Qt. Au lieu d'utiliser le framework multimédia de Qt, j'ai directement utilisé l'API de pilote de carte son de bas niveau de Windows (IMMDevice) pour collecter les données de la carte son, et utilisé des techniques de traitement du signal pour lisser les données audio. Le rythme audio a alors fait un bond qualitatif ! Le retard a disparu, et le rythme était assez bon. À ce moment-là, je me sentais vraiment génial. Mais un nouveau tournant est survenu :

Un camarade de promotion m'a demandé une question de programmation. Après l'avoir aidé, il m'a fait beaucoup de compliments. Je me suis alors senti fier et lui ai envoyé l'effet de Spec. Je me souviendrai toujours de cette conversation :

- Auteur : [Vidéo]
- Auteur : Cool, non ?
- Camarade : Qu'est-ce que c'est ?
- Auteur : Un logiciel que j'ai fait moi-même pour créer des spectres.
- Camarade : Oh, je sais que c'est un spectre.
- Auteur : J'ai passé beaucoup de temps à le faire, j'ai écrit des lignes de code...
- Camarade : Frère, pour être honnête, c'est vraiment trop mauvais. J'ai vu beaucoup de vidéos de spectres, beaucoup plus cool que le tien. Attends, je vais chercher.
- Camarade : [Vidéo]
- Camarade : Comment ça va ?

Après avoir vu la vidéo, je n'ai plus parlé, parce que je me sentais comme une blague. La vidéo était faite avec AE. L'effet de spectre de Spec était incomparable à celui d'AE. Cette nuit-là, je n'ai pas dormi. Je me demandais : "Comment AE arrive-t-il à réaliser un tel effet ?" J'ai alors recherché : "Oh, il a un effet de lueur." En cherchant davantage : "Il semble que la lueur consiste à flouter l'image, puis à l'éclaircir un peu." J'ai immédiatement commencé à chercher le principe du flou et à écrire un algorithme de flou. Après plusieurs heures de travail, j'ai découvert que flouter une image de 1000*1000 prenait plusieurs secondes. "Oh, oui, Qt n'a-t-il pas un effet de flou ? Je vais prendre ce code et l'utiliser." Après plusieurs heures de travail supplémentaires, cela prenait encore une ou deux secondes. "Ah, oui, ils disent toujours que le traitement d'image utilise OpenCV. Je vais essayer cela." Le temps était effectivement beaucoup plus rapide, mais environ 100 ms, ce qui ferait laguer le dessin en temps réel comme un diaporama. Finalement, j'ai终于了解到高性能绘图需要使用GPU. J'ai alors recherché sur Zhihu : "Quel outil utiliser pour le dessin GPU ?" J'ai continué à travailler jusqu'au lever du soleil. Mes colocataires sont allés en cours, et je me suis couché. Après cela, j'ai pris une semaine de repos. Lorsque je suis rentré chez moi, j'ai commencé mon apprentissage d'OpenGL. Cette période était très ennuyeuse : j'écrivais du code et m'endormais sur la table. Heureusement, j'ai demandé à mon frère de m'accompagner : pendant que j'apprenais OpenGL, il devait aussi lire des livres. Je n'ai pas utilisé QQ ou WeChat beaucoup, comme si j'avais disparu. Heureusement, j'ai persisté. Après avoir appris OpenGL, je n'ai pas commencé directement à coder, mais j'ai appris brièvement 3D Max et UE4. Je ne l'ai pas appris en profondeur, mais j'ai特别留意 leur architecture et leurs fonctionnalités. Finalement, j'ai commencé à écrire le code. Justement, j'étais en quatrième année, et ce projet est devenu mon projet de fin d'études. Ainsi, Spec tel qu'il est aujourd'hui a vu le jour.

Ce parcours a été rempli de sentiments. J'ai déjà été au plus bas, rencontré beaucoup de difficultés, mais heureusement, j'ai reçu l'aide de nombreuses personnes, ce qui m'a permis de rester fidèle à mon intention initiale et de me dépasser. J'ai déjà essayé d'échapper à la réalité, et les moments de lucidité étaient désespérés. Mais un jour, vous réaliserez : il y aura toujours quelqu'un, ou certaines personnes, pour lesquelles vous devrez vous tenir debout, parce que vous êtes leur espoir.

Sans rapport avec Spec, j'espère que les gens qui le verront pourront passer chaque jour avec optimisme et positivité.



**À propos du processus de développement de Spec, vous pouvez consulter ce lien :**

https://github.com/Italink/Italink-s-Undergraduate-Design

L'auteur est également en train de制作 des tutoriels sur OpenGL, que vous pouvez trouver dans la colonne.

En raison de mon travail, Spec ne pourra peut-être pas être mis à jour pour le moment, mais je ne l'abandonnerai jamais !

Quand j'aurai assez d'expérience à l'avenir, je ferai certainement un Spec meilleur et plus cool. À ce moment-là, il ne sera peut-être pas seulement affiché sur l'ordinateur 0.0