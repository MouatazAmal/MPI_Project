#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <math.h>
#include <mpi.h>
#include "FoxMPI.h"
#include "Matrice.h"

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
//Parce-que le modulo de C n'est pas un vrai modulo...
int mod(int a, int b){
    int r = a % b;
    return r < 0 ? r + b : r;
}
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

		printf("Valeur de A :\n");
		printMatrix(Me.A);
		printf("Valeur de B :\n");
		printMatrix(Me.B);
		printf("Valeur de C :\n");
		printMatrix(Me.C);
		printf("---------------------------------------\n");
	}
}
//-----------------------------------------------------
//Petite fonction pour broadcaster des matrices. Root va broadcast la matrice X sur le communicateur comm.
void theMatrixBroadcaster(Matrix X, int root, MPI_Comm comm){
	int i,j;
	
	MPI_Bcast(&X.Dim, 1,MPI_INT, root, comm);
	MPI_Barrier(comm);

	for(i=0;i<Me.A.Dim;i++){
		for(j=0;j<Me.A.Dim;j++){
			MPI_Bcast(&(X.M[i][j]), 1,MPI_INT, root, comm);
			MPI_Barrier(comm);
		}
	}

}
//-----------------------------------------------------
//Permet seulement à initialiser les matrices A et B de tous les processus : Le processus maitre (de rang_world = 0) lit les fichiers de matrice et envoie tout ça à tout le monde.
void initValue(char* filename_A, char* filename_B){
	int i,j;

	//LECTURE DES FICHIERS MATRICES PAR LE MAITRE
	if(Me.rank_world == 0) {
		Me.A = createMatrix(filename_A);
		Me.B = createMatrix(filename_B);

		/*printf("Je suis le Maitre (rang 0) et je vais envoyer la matrice A :\n");
		printMatrix(Me.A);
		printf("Ainsi que B :\n");
		printMatrix(Me.B);*/
	}
	
	//BROADCAST DE LA MATRICE A
	MPI_Bcast(&Me.A.Dim,1,MPI_INT, 0 ,G.grid_comm);
	MPI_Barrier(G.grid_comm);

	for(i=0;i<Me.A.Dim;i++){
		for(j=0;j<Me.A.Dim;j++){
			MPI_Bcast(&(Me.A.M[i][j]),1,MPI_INT, 0 ,G.grid_comm);
			MPI_Barrier(G.grid_comm);
		}
	}

//BROADCAST DE LA MATRICE B
	MPI_Bcast(&Me.B.Dim,1,MPI_INT, 0 ,G.grid_comm);
	MPI_Barrier(G.grid_comm);

	for(i=0;i<Me.B.Dim;i++){
		for(j=0;j<Me.B.Dim;j++){
			MPI_Bcast(&(Me.B.M[i][j]),1,MPI_INT, 0 ,G.grid_comm);
			MPI_Barrier(G.grid_comm);
		}
	}

	//INITIALISATION DE C
	Me.C.Dim = Me.A.Dim;

	//VERIFICATION [FACULTATIF]
	/*if(Me.rank_world == 8){
		printf("\n\n\n<QUELQUES MICRO SECONDES PLUS TARD...>\n\n\n");
		printf("Bonjour. Je suis l'esclave n°8, et j'ai lu A :\n");
		printMatrix(Me.A);
		printf("Ainsi que B :\n");
		printMatrix(Me.B);
		printf("Je n'ai pas vérifié mais je pense que mes confrères n°1 à n°7 ont reçu la même chose. Cordialement,\nEsclave n°8\n");
	}*/

	MPI_Barrier(G.grid_comm);
	fflush(stdout);
	
}
//-----------------------------------------------------
//ETAPE 1 : Broadcaster la matrice locale A des processus sur la diagonale k de la grille G, sur leur ligne
void broadcastDiagonal(int k){
	int i,j;
	int rank_processus_diagonal = mod(Me.coord_col[0]+k,G.dim); ///On remarque que rank_row = coord_col[0] quand le processus est dans la diagonale !

	//BROADCAST DE TOUTE LA MATRICE B
	for(i=0;i<Me.B.Dim;i++){
		for(j=0;j<Me.B.Dim;j++){
			MPI_Bcast(&(Me.B.M[i][j]),1,MPI_INT, rank_processus_diagonal ,G.row_comm);
			MPI_Barrier(G.grid_comm);
		}
	}	
}
//-----------------------------------------------------
void computation(){
	Matrix mulAB;

	mulAB = multiplyMatrix(Me.A,Me.B);
	Me.C = addMatrix(Me.C, mulAB);	
}
//-----------------------------------------------------
//Procède au shift de B : Sur une même colonne, Le processus [i,j] envoie sa matrice B à [i-1][j] et recoie celle de[i+1][j]
void doTheShift(){
	int i,j;

	//ENVOI/RECEPTION DU B LOCAL
	int rank_up = mod(Me.rank_col - 1,G.dim);
	int rank_down = mod(Me.rank_col + 1,G.dim);

	for(i=0;i<Me.B.Dim;i++){
		for(j=0;j<Me.B.Dim;j++){
			MPI_Sendrecv_replace(&Me.B.M[i][j], 1, MPI_INT, rank_up, 0, rank_down, 0, G.col_comm, MPI_STATUS_IGNORE);
			MPI_Barrier(G.grid_comm);
		}
	}
	
}
//-----------------------------------------------------
int main(int argc, char *argv[]){
	int k =0;

	//INITIALISATION
	MPI_Init(&argc, &argv); // Initialise  l'environnement MPI
    initGrid(); //Met en place la Grille + d'autres info supplémentaies utiles
	initValue(argv[1], argv[2]); //Initialisation des matrices : Tous les processus auront la même matrice A et B

	//ALGORITHME DE FOX
	for(k=0;k<G.dim;k++){
		broadcastDiagonal(0);
		computation();
		doTheShift();
		printStruct(8);
	}

	MPI_Finalize(); // cloturer l'environnement MPI

}
