# Projet MPI

# Authors 
	> Manal BENAISSA
	> Mouataz AMAL
## Première partie : Fox Algorithm
L'algorithme de Fox est COMPLETEMENT implémentée. L'implémentation de la gestion de mémoire (exercice 3) et l'analyse (exercie 4) ont été faites aussi pour cet algorithme.

### Compilation/Exécution
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

### Détails au sujet de la gestion de mémoire
MPI_Scatter et MPI_Gather n'ont pas été utilisé. En revanche, la matrice A et B sont subdivisés en autant de sous-matrices (elles même carrés) qu'il n'y a de processus dans la grille. Ainsi, pour une matrice A (et B) de taille 6x6, il est possible, avec seulement 4 processus (une grille 2x2) de paralléliser le calcul. En effet, le processus maitre (de rang 0) se charge de la subdivision. Dans cet exemple, il distribuera à chaque processus, une sous-matrice de taille 3x3. Après exécution du programme, nous pouvons voir comment la répartition a eu lieu :

~~~~
---------------------------------------
REPARTITION DES TACHES :
On va subdiviser les matrices A et B en sous-matrices, qui seront distribués à travers les processus (exemple pour A ci-dessous):
Matrice A initiale : 
1 2 3 8 9 5
5 6 7 4 2 6
9 0 1 2 5 7
0 9 7 3 8 4
2 7 5 0 8 1
1 8 9 5 4 9
Matrice [0,0] à envoyer au processus de rang (grille) 0:
1 2 3
5 6 7
9 0 1
Matrice [0,1] à envoyer au processus de rang (grille) 1:
8 9 5
4 2 6
2 5 7
Matrice [1,0] à envoyer au processus de rang (grille) 2:
0 9 7
2 7 5
1 8 9
Matrice [1,1] à envoyer au processus de rang (grille) 3:
3 8 4
0 8 1
5 4 9
---------------------------------------
Nombre de processus    : 4
Dimension de la grille : 2x2
---------------------------------------
Rang global          : 0
Rang dans la grille  : 0
Rang dans la ligne   : 0
Rang dans la colonne : 0

Coordonnées dans la grille  : [0,0]
Coordonnées dans la ligne   : [0,0]
Coordonnées dans la colonne : [0,0]

Valeur de A :
1 2 3 8 9 5
5 6 7 4 2 6
9 0 1 2 5 7
0 9 7 3 8 4
2 7 5 0 8 1
1 8 9 5 4 9
Valeur de B :
1 2 3 8 9 5
5 6 7 4 2 6
9 0 1 2 5 7
0 9 7 3 8 4
2 7 5 0 8 1
1 8 9 5 4 9
Valeur de C :
61 189 166 71 184 124
108 144 156 120 164 182
35 127 130 115 170 128
128 169 167 79 157 159
99 110 109 59 125 104
139 195 204 118 178 221
~~~~

De même, avec 9 processus, le maitre subdivisera en sous-matrices de taille 2x2. A la fin, tous les processus ont leur sous-matrices de C, qu'ils enverront au maitre pour que celui-ci réunisse tous les résultats, formant ainsi la matrice C complète.

### Analyse
L'analyse ici se fera sur la MEME machine. Il consistera à comparer le temps nécessaire pour l'algorithme de Fox, et le temps nécessaire pour un algorithme classique de produit matriciel, sans parallélisation. Les résultats obtenus pour des matrices 6x6 sont les suivants :

AVEC PARALLELISATION (4 processus):
~~~~
Temps d'initialisation :                        0.015015sec
Temps de distribution des tâches (scatter):     0.001000sec
Temps de l'algorithme de fox :                  0.000366sec
Temps de rassemblement des résultats (gather) : 0.000919sec
TEMPS TOTAL :                                   0.017300sec
~~~~

AVEC PARALLELISATION (9 processus):
~~~~
Temps d'initialisation :                        0.018695sec
Temps de distribution des tâches (scatter):     0.001364sec
Temps de l'algorithme de fox :                  0.002370sec
Temps de rassemblement des résultats (gather) : 0.000081sec
TEMPS TOTAL :                                   0.022510sec
~~~~

SANS PARALLELISATION : 
~~~~
Temps d'initialisation :    0.000460sec
Temps de calcul :           0.000113sec
TEMPS TOTAL :               0.000573sec
~~~~

Il aurait été plus intéressant de tester avec des matrices bien plus grandes (50x50 minimum, 1000x1000 dans l'idéal), mais nos machines/codes ne l'ont pas permis (nous manquions de temps pour paufiner les analyses)



## Deuxième partie : Cannon Matrix product algorithm

Dans cet exercice nous avons mis en place l’algorithme, cependant dans notre programme chaque processeur de la grille ne prends en compte qu’une valeur et pas de matrices.


### Pour compiler : 
	exécuter le makefile dans le répertoire Exercices2/
	> make

### Pour exécuter :
	>  mpirun -n <NB_PROCESSUS> ./CannonAlgo

### Optimisations : 
L’optimisation que nous aurions pu ajouter est la gestion des matrices par les processeurs de la grille.

### Exemple : 
Nous avons exécuté notre programme avec 9 processeurs.
La grille est de dimensions [3,3]
La matrice A :
~~~~
	2 2 2
	2 2 2
	2 2 2
~~~~

La matrice B :
~~~~
	3 3 3
	3 3 3
	3 3 3
~~~~

output : 
La matrice C
~~~~
	18 18 18
	18 18 18
	18 18 18
~~~~