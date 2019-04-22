# Projet MPI

## Première partie : Fox Algorithm
L'algorithme de Fox est COMPLETEMENT implémentée (je verrais plus tard pour les optimisations)

Pour compiler :
> make

Pour exécuter :
> mpirun -n <NB_PROCESSUS> ./FoxMPI <FICHIER_MATRICE_A> <FICHIER_MATRICE_B>

Il suffit de donner en paramètre deux fichiers de matrices. Ces fichiers doivent suivre ce format : 
- La première ligne contient une seule valeur : la dimension de la matrice
- Puis la matrice est retranscrite ligne par ligne, où chaque élément est séparé par un espace.

Exemple :
>3
>
>1 2 3
>
>4 5 6
>
>7 8 9

ATTENTION : 
- Les matrices A et B doivent être CARRES et de MEME DIMENSION
- Le nombre de processus doit être un CARRE PARFAIT p, tel que sqrt(p) soit un MULTIPLE de dim(A) et dim(B)

Exemple : 
- 9 processus (grille 3x3 de processus) pour des matrices de dimension 6 est bon.
- 4 processus (grille 2x2 de processus) pour des matrices de dimension 6 est bon.
- 9 processus pour des matrices de dimension 4 n'est PAS BON.
