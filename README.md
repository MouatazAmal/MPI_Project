# Projet MPI

## Première partie : Fox Algorithm
J'ai fait ma petite version à part : FoxMPI.c/h Pour l'instant, il initie la grille "proprement".
Elle aura bientôt besoin d'une librairie de gestion de matrices aussi : Matrice.c/h

Pour compiler :
> make

Pour exécuter :
> mpirun -n <NB_PROCESSUS> ./FoxMPI <RANG>

Le paramètre <RANG> permet d'avoir des infos sur le processus de rang global (ou rang de la grille) <RANG>. Si <RANG> = 1000, tous les processus affichent leur infos.
