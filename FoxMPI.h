#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <math.h>
#include <mpi.h>

//---------------------------------------------------
/*Le but de cet exercice est de permettre de paralléliser la multiplication de 2 matries A et B, en
utilisant MPI. Il nous faut une grille (2D) de processus qui permettra cette parallélisation.
La librairie Matrice.h nous servira pour les calculs de base sur des matrices.*/
//---------------------------------------------------

//GRID : Structure définissant lensemble de la grille et des noeuds (processus) q'il contient
typedef struct {
    int nbProcessus;    //nombre total de processus
    int dim;            //dimension de la grille
    MPI_Comm grid_comm; //Communicateur GRILLE
    MPI_Comm row_comm;  //Communicateur LIGNE
    MPI_Comm col_comm;  //Communicateur COLONNE
    int k; 
} Grid;

//NODE : Structure donnant les infos propre au processus actuel : son rang dans un communicateur donné, ses coordonnées...
typedef struct {
    int rank_world; //rang du processus dans MPI_COMM_WORLD
    int rank_grid;  //rang du processus dans G.grid_comm
    int rank_row;   //rang du processus dans G.row_comm
    int rank_col;   //rang du processus dans G.col_comm

    int coord_grid[2];  //coordonnées du processus dans G.grid_comm
    int coord_row[2];   //coordonnées du processus dans G.row_comm
    int coord_col[2];   //coordonnées du processus dans G.col_comm

    int a;  //Valeur LOCAL a du processus
    int b;  //Valeur LOCAL b du processus
    int c;  //Valeur LOCAL b du processus
} Node;

//---------------------------------------------------
void initGrid();
void printStruct(int rank);
void initValue();
void broadcastDiag();
