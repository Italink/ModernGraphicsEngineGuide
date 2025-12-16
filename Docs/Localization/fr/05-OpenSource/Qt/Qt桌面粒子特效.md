---
comments: true
---

# Effets de particules de bureau Qt

- Dépôt Github : https://github.com/Italink/DesktopParticlesEffect

J'ai récemment regardé *Nezha* et découvert un effet de particules très spectaculaire :

![img](../../../../05-OpenSource/Qt/Resources/7cee99b178e3913b3ffe1f74a322ca91.gif)

J'ai eu l'idée de l'implémenter moi-même, voici le résultat final :

![img](../../../../05-OpenSource/Qt/Resources/6f459e67df921a7cf9b8862cb8dd54d7.gif)

## Outils utilisés

Maîtriser au moins un outil GUI : ici, j'ai utilisé Qt5 + QtCreator

## Compétences requises

- Mécanismes de dessin
- Multithreading (mise à jour en temps réel de l'interface)
- Modification des propriétés de fenêtre Windows (transparence de fenêtre, masquage des bordures, perméabilité à la souris)

## Mécanisme d'animation

- Points : L'animation contient de nombreux points flottants (en réalité de petits cercles) qui se déplacent à différentes vitesses et dans différentes directions. Pour éviter que les points ne sortent de l'écran, nous ajoutons un mécanisme de collision : quand un point touche l'écran, sa direction de mouvement change (suivant le mécanisme de réflexion de la lumière) et nous lui attribuons une vitesse aléatoire lors de la collision.
- Lignes : En détectant l'ensemble des points ci-dessus, nous dessinons une ligne quand la distance entre deux points atteint une certaine plage, et la transparence de la ligne est associée à sa longueur.
- Triangles : Après avoir dessiné une ligne, nous parcourons l'ensemble des points pour vérifier si un triangle peut être formé. Si oui, nous le dessinons et associons sa transparence.
- Déplacement de la souris : Nous définissons un point pour suivre la position de la souris, de sorte que la position de la souris peut également dessiner des lignes et des triangles. Nous enregistrons l'historique de la trajectoire de la souris : quand la vitesse de déplacement de la souris atteint une certaine valeur, nous plaçons aléatoirement un point de l'ensemble sur l'historique de la trajectoire.

## Configuration

![img](../../../../05-OpenSource/Qt/Resources/c8d801c45bcd904e7ac65427701242a2.png)

## Code de dessin

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
    for(unsigned int i=0;i<points.size();++i){      // Attention à la suppression des doublons
        for(unsigned int j=i+1;j<points.size();++j){
            if(points[i].getDistance(points[j])<Config::maxLen_of_line){
                c.setAlpha(Config::maxLen_of_line-points[i].getDistance(points[j]));
                pen.setBrush(c);
                painter.setPen(pen);
                painter.drawLine(points[i].getX(),points[i].getY(),points[j].getX(),points[j].getY());      // Dessiner la ligne
                c.setAlpha((Config::maxLen_of_line-points[i].getDistance(points[j]))/3);
                painter.setBrush(c);
                for(int k=j+1;k<points.size();++k){
                    if(points[k].getDistance(points[i])<Config::maxLen_of_line&&points[k].getDistance(points[j])<Config::maxLen_of_line){
                        p.setPoints(3,int(points[i].getX()),int(points[i].getY()),int(points[j].getX()),int(points[j].getY()),int(points[k].getX()),int(points[k].getY()));
                        painter.drawPolygon(p);                                 // Dessiner le triangle
                    }
                }
            }
 
        }
        painter.setBrush(QColor(252,251,243));
        painter.setPen(Qt::NoPen);
        if(i)           // Dessiner le point, le premier point est utilisé pour la souris
            painter.drawEllipse(points[i].getRect());
    }
}
```

## Code d'exécution des points

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