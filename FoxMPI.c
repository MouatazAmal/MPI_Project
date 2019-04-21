#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <math.h>
#include <mpi.h>
#include "FoxMPI.h"
#include "Matrice.h"

#define ROOT 0

// Compilation : make

// Exécution : mpirun -n 9 ./FoxMPI matrice1.txt
// Le 2 indique le nombre de processus à faire tourner
// On indique ensuite sur quel machines on veut faire tourner les processus [facultatif !]

//-----------------------------------------------------
/*
Garder en tête que ce main est exécuté par chaque processus de la grille G.

Inspiration : https://github.com/dmitry64/foxmpi/blob/master/main.cpp
*/
//-----------------------------------------------------
Grid G; //Grille (2D Torus) de processus
Node Me;  //Info du Processus actuel

//-----------------------------------------------------
//INITGRID : Permet de mettre en place la grille et les différents communicateurs, ainsi que les infos des processus 
void initGrid(){
	
	//INITIALISATION DE BASE
	MPI_Comm_size(MPI_COMM_WORLD,&(G.nbProcessus)); //Donne le nombre de processus dans le communicateur donné
	MPI_Comm_rank(MPI_COMM_WORLD, &(Me.rank_world)); // Donne le rang du processus dans MPI_COMM_WORLD
	G.dim = (int) sqrt((double) G.nbProcessus); //La dimension de la grille dépendra du nombre de processus dispo, je veux UN seul processus par noeud

	//GESTION D'ERREUR
	if((Me.rank_world == 0)&&(G.dim*G.dim != G.nbProcessus)){
		printf("\nERREUR : Donnez un nombre carré de processus à lancer pour une meilleure répartition dans la grille.\nExemple : mpirun -n 9 ./FoxMPI <autres paramètres>\n\n");
		MPI_Finalize();
		exit(0);
	}
	else if(G.dim*G.dim != G.nbProcessus){
		MPI_Finalize();
		exit(0);
	}
	
    //CREATION DES COMMUNICATEURS : grid_comm, la grille de base
	int Dims[2] = {G.dim,G.dim}; //Je veux que 1 case = 1 processus, donc dans chaque dimension il y a G.dim processus
	int period[2] = {1,1}; //indique si le ableau est périodique (1) ou non (0) pour chaque dimension (cf https://www.mcs.anl.gov/research/projects/mpi/tutorial/gropp/node70.html)
	MPI_Dims_create(G.nbProcessus,2,Dims); //Aide à définir le nombre approprié de processus dans chaque noeud, va sans doute retourner {G.dim,G.dim}.
	MPI_Cart_create(MPI_COMM_WORLD, 2, Dims, period, 1,&(G.grid_comm)); //Création de grid_comm

	MPI_Comm_rank(G.grid_comm,&(Me.rank_grid)); //le rang diffère entre chaque communicateur ! Il au donc garder ces nouveaux rangs.
	MPI_Cart_coords(G.grid_comm, Me.rank_grid, 2, Me.coord_grid); //Coordonnées du processus de rang Me.rank_grid dans la GRILLE

	//CREATION DES COMMUNICATEURS : row_comm, les lignes de notre grille
	int row_def[2] = {0,1}; //La grille est une matrice M[i][j] où i forme les colonnes et j les lignes
	MPI_Cart_sub(G.grid_comm,row_def, &(G.row_comm));//On va partitionner notre grille en lignes, créant ainsi le communicateur row_comm
	MPI_Comm_rank(G.row_comm,&(Me.rank_row));
	MPI_Cart_coords(G.row_comm, Me.rank_row, 2, Me.coord_row); 
	
	//CREATION DES COMMUNICATEURS : col_comm, les colonnes de notre grille
	int col_def[2] = {1,0};
	MPI_Cart_sub(G.grid_comm,col_def, &(G.col_comm));
	MPI_Comm_rank(G.col_comm,&(Me.rank_col));
	MPI_Cart_coords(G.col_comm, Me.rank_col, 2, Me.coord_col); 
}
//-----------------------------------------------------
//Va afficher les info du processus de rang "rang" dans le communicateur global
void printStruct(int rank){
	
	if((Me.rank_world == rank)||(rank == 1000)){
		printf("---------------------------------------\n");
		printf("Nombre de processus    : %d\n", G.nbProcessus);
		printf("Dimension de la grille : %dx%d\n", G.dim, G.dim);
		printf("---------------------------------------\n");
		printf("Rang global          : %d\n", Me.rank_world);
		printf("Rang dans la grille  : %d\n", Me.rank_grid);
		printf("Rang dans la ligne   : %d\n", Me.rank_row);
		printf("Rang dans la colonne : %d\n\n", Me.rank_col);

		printf("Coordonnées dans la grille  : [%d,%d]\n", Me.coord_grid[0], Me.coord_grid[1]);
		printf("Coordonnées dans la ligne   : [%d,%d]\n", Me.coord_row[0], Me.coord_row[1]);
		printf("Coordonnées dans la colonne : [%d,%d]\n\n", Me.coord_col[0], Me.coord_col[1]);

		printf("Valeur de a : %d\n", Me.a);
		printf("Valeur de b : %d\n", Me.b);
		printf("Valeur de c : %d\n", Me.c);
		printf("---------------------------------------\n");
	}
}
//-----------------------------------------------------
//Permet seulement à initialiser les valeurs a,b et c de tous les processus
void initValue(){
	
	if(Me.coord_grid[0] == Me.coord_grid[1]){
		
		Me.a = Me.rank_grid; //Je donne une valeur au pif, ici le rang de grille.
	}
	else{
		Me.a = -1;
	}
}
//-----------------------------------------------------
void broadcastDiag(){

	int k =0;
	for(k=0;k<G.dim;k++){
		int proc_diag = (Me.coord_col[0]+k)%G.dim;
		MPI_Bcast(&(Me.a),1,MPI_INT, proc_diag ,G.row_comm); ///On remarque que rank_row = coord_col[0] quand le processus est dans la diagonale !
		MPI_Barrier(MPI_COMM_WORLD);
		
	}
}
//-----------------------------------------------------
void ShiftOfB(){
	int received_B;

	int dest = Me.rank_col - 1;
	if (dest == -1) {
		dest = G.dim -1;
	}
	
	if (Me.rank_col != ROOT) {
		MPI_Recv(&received_B, 1, MPI_INT, (Me.rank_col + 1)%G.dim, 0, G.col_comm,MPI_STATUS_IGNORE);
	}

	MPI_Send(&Me.b, 1, MPI_INT, dest,0, G.col_comm);

	if (Me.rank_col == ROOT) {
    	MPI_Recv(&received_B, 1, MPI_INT, (Me.rank_col + 1)%G.dim, 0,G.col_comm,MPI_STATUS_IGNORE);

	}

	MPI_Barrier(MPI_COMM_WORLD);
	Me.b = received_B;
	
}
//-----------------------------------------------------
int main(int argc, char *argv[]){

	/*Matrix A, B, C;
	A = createMatrix(argv[1]);
	B = createMatrix(argv[2]);
	C.Dim = A.Dim; //A la fin, C = A*B*/

	MPI_Init(&argc, &argv); // Initialise  l'environnement MPI
    initGrid(); //Met en place la Grille + d'autres info supplémentaies utiles
	//printStruct(atoi(argv[1])); //Connaitre les infos du processus de rang argv[1]

	initValue();

	broadcastDiag();
	//printStruct(atoi(argv[1]));
	ShiftOfB();
	printf("process %d : B = %d\n", Me.rank_col, Me.b);

	MPI_Finalize(); // cloturer l'environnement MPI

}
