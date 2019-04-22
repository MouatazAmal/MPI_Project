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
Master MeMaster; //Info utile au processus 0 (les autres s'en foutent de cette structure)
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
void printAllStruct(int rank){
		
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

		//Cette partie sert surtout au maitre, se sont les matrices COMPLETES
		printf("Valeur de A :\n");
		printMatrix(MeMaster.A);
		printf("Valeur de B :\n");
		printMatrix(MeMaster.B);
		printf("Valeur de C :\n");
		printMatrix(MeMaster.C);
		printf("---------------------------------------\n");
		
	}
}
//-----------------------------------------------------
//Va afficher les info du processus de rang "rang" dans le communicateur global
void printStruct(int rank){
		
	if((Me.rank_world == rank)||(rank == 1000)){
		printf("---------------------------------------\n");
		printf("Rang global          : %d\n", Me.rank_world);

		//Cette partie sert à tous les processus : se sont les fameux blocs pour les calculs !
		printf("Valeur de a :\n");
		printMatrix(Me.a);
		printf("Valeur de b :\n");
		printMatrix(Me.b);
		printf("Valeur de c :\n");
		printMatrix(Me.c);

		printf("---------------------------------------\n");
		
	}
}
//-----------------------------------------------------
//Permet seulement à initialiser les matrices A et B de tous les processus : Le processus maitre (de rang_world = 0) lit les fichiers de matrice et envoie tout ça à tout le monde.
void initValue(char* filename_A, char* filename_B){
	int i,j,k,l;
	
	//LECTURE DES FICHIERS MATRICES PAR LE MAITRE
	if(Me.rank_world == 0) {
		//RECUPERATION DES MATRICES
		MeMaster.A = createMatrix(filename_A);
		MeMaster.B = createMatrix(filename_B);
		MeMaster.C.Dim = MeMaster.A.Dim;

		MeMaster.bloc_dim = MeMaster.A.Dim/G.dim; //On va répartir le travail à travers tous les processus. Chaque processus va recevoir une sous-matrice de dimension bloc_dim
		
		//CREATION D'UNE MATRICE DE SOUS-MATRICES, DE MEME DIMENSION QUE LA GRILLE
		Matrix repartition_des_taches_A[G.dim][G.dim];
		Matrix repartition_des_taches_B[G.dim][G.dim];

		//Pour chaque bloc de repartition_des_taches...
		for(i=0;i<G.dim;i++){ 
			for(j=0;j<G.dim;j++){
				//...Je construis la sous-matrice de A et B 
				repartition_des_taches_A[i][j].Dim = MeMaster.bloc_dim;
				repartition_des_taches_B[i][j].Dim = MeMaster.bloc_dim;

				for(k=0;k<MeMaster.bloc_dim;k++){
					for(l=0;l<MeMaster.bloc_dim;l++){
						repartition_des_taches_A[i][j].M[k][l] = MeMaster.A.M[k+i*(G.dim-1)][l+j*(G.dim-1)];	
						repartition_des_taches_B[i][j].M[k][l] = MeMaster.B.M[k+i*(G.dim-1)][l+j*(G.dim-1)];	
					}
				}
			}
		}


		//ENVOI DES BLOCS A CHAQUE PROCESSUS(1 processus = 1 bloc)
		for(i=0;i<G.dim;i++){
			for(j=0;j<G.dim;j++){
				if(i+j != 0){
					
					//ENVOI DIMENSION
					Me.a.Dim = repartition_des_taches_A[i][j].Dim;
					MPI_Send(&(Me.a.Dim), 1, MPI_INT, i*G.dim+j, 0, G.grid_comm);

					Me.b.Dim = repartition_des_taches_B[i][j].Dim;
					MPI_Send(&(Me.b.Dim), 1, MPI_INT, i*G.dim+j, 0, G.grid_comm);

					//ENVOI BLOC
					for(k=0;k<Me.a.Dim;k++){
						for(l=0;l<Me.a.Dim;l++){
							Me.a.M[k][l] = repartition_des_taches_A[i][j].M[k][l];
							MPI_Send(&(Me.a.M[k][l]), 1, MPI_INT, i*G.dim+j, 0, G.grid_comm);

							Me.b.M[k][l] = repartition_des_taches_B[i][j].M[k][l];
							MPI_Send(&(Me.b.M[k][l]), 1, MPI_INT, i*G.dim+j, 0, G.grid_comm);

						}
					}
				}
			}
		}

		//LE MAITRE RESTITUE SON BLOC A LUI
		Me.origin_a.Dim = Me.a.Dim;

		for(k=0;k<Me.a.Dim;k++){
			for(l=0;l<Me.a.Dim;l++){
				Me.a.M[k][l] = repartition_des_taches_A[0][0].M[k][l];
				Me.origin_a.M[k][l] = Me.a.M[k][l];
				Me.b.M[k][l] = repartition_des_taches_B[0][0].M[k][l];
			}
		}

	}

	else{
		MPI_Recv(&(Me.a.Dim), 1, MPI_INT, 0, 0, G.grid_comm, MPI_STATUS_IGNORE);
		MPI_Recv(&(Me.b.Dim), 1, MPI_INT, 0, 0, G.grid_comm, MPI_STATUS_IGNORE);
		Me.origin_a.Dim = Me.a.Dim;

		for(k=0;k<Me.a.Dim;k++){
			for(l=0;l<Me.a.Dim;l++){
				MPI_Recv(&(Me.a.M[k][l]), 1, MPI_INT, 0, 0, G.grid_comm, MPI_STATUS_IGNORE);
				MPI_Recv(&(Me.b.M[k][l]), 1, MPI_INT, 0, 0, G.grid_comm, MPI_STATUS_IGNORE);
				Me.origin_a.M[k][l] = Me.a.M[k][l];
			}
		}
	}

	Me.c.Dim = Me.a.Dim;

	MPI_Barrier(G.grid_comm);
	fflush(stdout);
	
}
//-----------------------------------------------------
//ETAPE 1 : Broadcaster la matrice locale A des processus sur la diagonale k de la grille G, sur leur ligne
void broadcastDiagonal(int k){
	int i,j;
	int rank_processus_diagonal = mod(Me.coord_col[0]+k,G.dim); ///On remarque que rank_row = coord_col[0] quand le processus est dans la diagonale !

	//BROADCAST DE TOUTE LA MATRICE A
	for(i=0;i<Me.a.Dim;i++){
		for(j=0;j<Me.a.Dim;j++){
			MPI_Bcast(&(Me.a.M[i][j]),1,MPI_INT, rank_processus_diagonal ,G.row_comm);
			MPI_Barrier(G.grid_comm);
		}
	}	
}
//-----------------------------------------------------
//ETAPE 2 : Le calcul...
void computation(){
	Matrix mulAB;

	mulAB = multiplyMatrix(Me.a,Me.b);
	Me.c = addMatrix(Me.c, mulAB);	

}
//-----------------------------------------------------
//Procède au shift de B : Sur une même colonne, Le processus [i,j] envoie sa matrice B à [i-1][j] et recoie celle de[i+1][j]
void doTheShift(){
	int i,j;

	//ENVOI/RECEPTION DU B LOCAL
	int rank_up = mod(Me.rank_col - 1,G.dim);
	int rank_down = mod(Me.rank_col + 1,G.dim);

	for(i=0;i<Me.b.Dim;i++){
		for(j=0;j<Me.b.Dim;j++){
			MPI_Sendrecv_replace(&Me.b.M[i][j], 1, MPI_INT, rank_up, 0, rank_down, 0, G.col_comm, MPI_STATUS_IGNORE); //Cette fonction est magique.
			MPI_Barrier(G.grid_comm);
		}
	}
	
}
//-----------------------------------------------------
//Permet juste de restaurer les vraies valeurs de la matrice a, avant le broadcast. En effet, après broadcast, l'ancienne valeur de a est perdue.
void restoreAOrigin(){
	int i,j;
	for(i=0;i<Me.a.Dim;i++){
		for(j=0;j<Me.a.Dim;j++){
			Me.a.M[i][j] = Me.origin_a.M[i][j];
		}
	}
}
//-----------------------------------------------------
//A cette étape, tous les processus ont fini leur calculs. Il ne reste plus qu'à assembler tous leur bloc pour former la matrice C finale.
void closureFox(){
	int i,j,k,l;

	if(Me.rank_world == 0) {
				
		//CREATION D'UNE MATRICE DE SOUS-MATRICES, DE MEME DIMENSION QUE LA GRILLE, POUR RECUPERER LES RESULTATS
		Matrix repartition_des_taches_C[G.dim][G.dim];

		//RECEPTION DE TOUS LES RESULTATS
		for(i=0;i<G.dim;i++){ 
			for(j=0;j<G.dim;j++){

				for(k=0;k<Me.a.Dim;k++){
					for(l=0;l<Me.a.Dim;l++){
						if(i+j != 0){
							MPI_Recv(&(Me.c.M[k][l]), 1, MPI_INT, i*G.dim+j, 0, G.grid_comm, MPI_STATUS_IGNORE);
						}
						repartition_des_taches_C[i][j].M[k][l] = Me.c.M[k][l];
						
					}
				}

			}
		}

		//Pour chaque bloc de repartition_des_taches...
		for(i=0;i<G.dim;i++){ 
			for(j=0;j<G.dim;j++){
				//...Je reconstruis la matrice de C 

				for(k=0;k<MeMaster.bloc_dim;k++){
					for(l=0;l<MeMaster.bloc_dim;l++){
						MeMaster.C.M[k+i*(G.dim-1)][l+j*(G.dim-1)] = repartition_des_taches_C[i][j].M[k][l];
					}
				}
			}
		}

	}
	else{
		//Ce qu'envoient les autres processus (leur sous-matrice Me.c)
		for(k=0;k<Me.c.Dim;k++){
			for(l=0;l<Me.c.Dim;l++){
					MPI_Send(&(Me.c.M[k][l]), 1, MPI_INT, 0, 0, G.grid_comm);

			}
		}
	}
	MPI_Barrier(G.grid_comm);


}
//-----------------------------------------------------
int main(int argc, char *argv[]){
	int k=0;

	//INITIALISATION
	MPI_Init(&argc, &argv); // Initialise  l'environnement MPI
    initGrid(); //Met en place la Grille + d'autres info supplémentaies utiles
	initValue(argv[1], argv[2]); //Initialisation des matrices : Tous les processus auront la même matrice A et B


	//ALGORITHME DE FOX
	for(k=0;k<G.dim;k++){
		//RESTAURATION DU a D'ORIGINE
		restoreAOrigin();
		broadcastDiagonal(k);
		computation();
		doTheShift();
	}

	//RECUPERATION DES RESULTATS DE TOUS LES PROCESSUS PAR LE MAITRE
	closureFox();
	printAllStruct(0);

	MPI_Finalize(); // cloturer l'environnement MPI
	return 0;

}
