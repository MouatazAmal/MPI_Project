#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <math.h>
#include <mpi.h>
#include "FoxMPI.h"
#include "Matrice.h"

#define ROOT 0


Grid G; //Grille (2D Torus) de processus
Node Me;  //Info du Processus actuel

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


void initValue(){
	Me.a = 2;
	Me.b = 3;
	Me.c = 0;
}

void shiftA(){

	if (Me.coord_grid[0] >= 1){
		int destination = Me.rank_row - 1;
		int source = (Me.rank_row +1)%G.dim;
		if (destination < 0 ) destination = G.dim - 1;

		MPI_Sendrecv_replace(&Me.a, 1,MPI_INT,destination,0,source,0,G.row_comm,MPI_STATUS_IGNORE);
	}

	MPI_Barrier(G.row_comm);

}

void shiftB(){

	if (Me.coord_grid[1] >= 1){
		int destination = Me.rank_col - 1;
		int source = (Me.rank_col +1)%G.dim;
		if (destination < 0 ) destination = G.dim - 1;

		MPI_Sendrecv_replace(&Me.b, 1,MPI_INT,destination,0,source,0,G.col_comm,MPI_STATUS_IGNORE);
	}

	MPI_Barrier(G.col_comm);

}

void matrixpreskewingA(){

    int i = 1;  
    for(i=1; i <= Me.rank_col; i++)
	{
		shiftA();
	}
	
    MPI_Barrier(G.row_comm);

}

void matrixpreskewingB(){

    int i = 1;  
    for(i=1; i <= Me.rank_row; i++)
	{
		shiftB();
	}
	
    MPI_Barrier(G.col_comm);

}

void localComputation(){
	Me.c = Me.c + Me.a*Me.b;
}


int main(int argc, char *argv[]){

	/*Matrix A, B, C;
	A = createMatrix(argv[1]);
	B = createMatrix(argv[2]);
	C.Dim = A.Dim; //A la fin, C = A*B*/

	MPI_Init(&argc, &argv); // Initialise  l'environnement MPI
    initGrid(); //Met en place la Grille + d'autres info supplémentaies utiles
	initValue();

	matrixpreskewingA();
	matrixpreskewingB();

	int k = 0;
	for(k=0 ; k<G.dim; k++)
	{
		localComputation();
		shiftA();
		shiftB();
	}

	matrixpreskewingA();
	matrixpreskewingB();
	
	printf("Process %d : C = %d\n",Me.rank_grid, Me.c);

	MPI_Finalize(); // cloturer l'environnement MPI


}
