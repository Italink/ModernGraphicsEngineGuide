---
comments: true
---

# Sélecteur de couleur d'écran Qt

- Adresse : https://github.com/Italink/ColorPicker.git

![img](../../../../05-OpenSource/Qt/Resources/c42c52b30c698cbbd842c6c8f6167e12.gif)

## Détails du programme

Ajustement automatique de la position d'affichage de la loupe

Changement automatique de la couleur de la bordure

## Principe

- D'abord, couvrir l'ensemble de l'écran avec une fenêtre sans bordure à haute transparence (ici, j'ai utilisé la couleur (255, 255, 255) avec une transparence de 1 (plage [0, 255])).
- Mettre à jour en temps réel la position de la souris via un minuteur ou un thread. Notez qu'il ne faut pas utiliser l'événement de mouvement de la souris, car si vous déclenchez la capture de couleur via le mouvement de la souris, vous ne pourrez pas capturer la couleur de vidéos dynamiques.
- Utiliser une fonction de capture d'écran pour prendre une capture de l'ensemble de la fenêtre, puis sélectionner un rectangle centré sur la souris. La taille que j'ai choisie est 10*7, tandis que la loupe est de 100*70, donc le facteur d'agrandissement est 10. Placer cette partie du rectangle graphique dans la loupe et sélectionner la couleur du pixel à la position de la souris.
- La couleur du pixel sélectionné aura une déviation en raison de la fenêtre transparente, il faut donc restaurer la couleur originale selon l'algorithme de transparence.
- Principe de transparence : Supposons que B soit la couleur transparente, a la transparence, A la couleur du fond et C la couleur finale affichée, alors 255*C = a*B + (255-a)*A.
- Il suffit de calculer A selon la formule pour restaurer la couleur.
- Lorsque la souris est cliquée, fermer la fenêtre transparente et envoyer le signal de couleur.

## Instructions d'utilisation

Il suffit d'importer les fichiers de classe et de créer une instance ColorPicker dans la fenêtre souhaitée. Comme elle hérite de QWidget, appelez la fonction show pour l'afficher.

Vous pouvez obtenir la couleur sélectionnée via le signal connectable `void QColorPicker::colorSelect(const QColor&)`.

Notez qu'il faut importer le fichier d'icône de connexion, sinon la capture de couleur peut ne pas être précise.

## Instructions de configuration

Situé dans mousedropper.cpp

``` c++
const QSize winSize(100,100);       //Taille de la fenêtre
const int grabInterval=50;          //Fréquence de rafraîchissement
const int magnificationTimes=10;    //Facteur d'agrandissement
const double split=0.7;             //Division
const int sizeOfMouseIcon=20;       //Taille de l'icône de souris : matériel d'icône
```