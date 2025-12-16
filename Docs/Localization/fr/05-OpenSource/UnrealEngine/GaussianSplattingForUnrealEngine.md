---
comments: true
---
# Gaussian Splatting For Unreal Engine

- Adresse du dépôt : https://github.com/Italink/GaussianSplattingForUnrealEngine/

**GaussianSplattingForUnrealEngine** est un plugin Unreal Engine qui permet de convertir facilement les graphiques dans Unreal en nuages de points 3D gaussiens de haute qualité :

![3dgs](../../../../05-OpenSource/UnrealEngine/Resources/3dgs.gif)

![image-20250118162622141](../../../../05-OpenSource/UnrealEngine/Resources/image-20250118162622141.png)

Ce plugin prend en charge les fonctionnalités suivantes :

- Fournit des outils d'éditeur simples et faciles à utiliser pour réaliser :
    - Capture de graphiques
    - Reconstruction de nuage de points sparse
    - Entraînement gaussien
- Prend en charge l'importation de nuages de points gaussiens (`*.ply`), avec rendu dans Unreal en utilisant des **GPU Particles** ou des **Static Meshes** comme supports
- Rapproche les 3D Gaussiens de la production industrielle en fournissant des mécanismes très utiles :
    - Coupe de profondeur精细化 (Trimming de profondeur精细化) : Élimine efficacement les points de bruit flottants
    - Stratégie LOD basée sur la taille de l'écran : Régule efficacement le nombre de particules et la mémoire en se basant sur les caractéristiques des points gaussiens

Les 3D Gaussiens présentent les avantages et inconvénients suivants :

- Avantages :
    - L'expression basée sur les **points** est plus adaptée à la création d'effets de particules que les triangles
    - La reconstruction basée sur la reconnaissance de caractéristiques d'image a généralement un meilleur effet de simplification que la simplification de maillage traditionnelle, elle est donc très adaptée à la création d'agents visuels pour de grandes zones
    - Aucune donnée de texture, seulement des données de sommets basées sur des caractéristiques ; la propriété différentiable permet également un rognage et une compression libres
- Inconvénients :
    - Utilise un mode de superposition semi-transparente pour le rendu, nécessite un tri semi-transparent des primitives et a un OverDraw élevé
    - Ne restaure que l'expression de couleur visuelle de l'objet, ne peut pas ou difficilement restaurer les propriétés physiques réelles de l'objet, donc ne peut pas créer d'effets d'éclairage et d'ombres dynamiques