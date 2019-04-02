#include <stdio.h>
#include <string.h>
#include <mpi.h>
#define N 3
#define ROOT 0

// Compilation : mpicc -o Exercice1 Exercice1.c

// Exécution : mpirun -n 100 ./Exercice1
// Le 2 indique le nombre de processus à faire tourner
// On indique ensuite sur quel machines on veut faire tourner les processus [facultatif !]

int main(int argc, char *argv[]){
		
	int my_rank;
	MPI_Status status;
	MPI_Comm grid_comm,row_comm,line_comm;
	int gridRank, rRank, lRank;
	int coordinates[2];
	int period[2] = {1,1};

	MPI_Init(&argc, &argv); // Initialise l'env d'exécution
	// Creating ranks of threads.
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	int nbProcess;
	// affecting to nbProcess the number fo process
	MPI_Comm_size(MPI_COMM_WORLD,&nbProcess);
	int Dims[2] = {3,3};
	MPI_Dims_create(nbProcess,2,Dims);
 	MPI_Cart_create(MPI_COMM_WORLD, 2, Dims, period, 1,&grid_comm);

 	MPI_Comm_rank(grid_comm,&gridRank);

	MPI_Cart_coords(grid_comm, gridRank, 2, coordinates);
	int row = coordinates[0];
	int col = coordinates[1];

	int row_def[2] = {1,0};
	MPI_Cart_sub(grid_comm,row_def, &row_comm);
	int col_def[2] = {0,1};
	MPI_Cart_sub(grid_comm,col_def, &line_comm);
	
	MPI_Comm_rank(row_comm,&rRank);
	MPI_Comm_rank(line_comm,&lRank);

	if (gridRank == 0) printf("The created Grid is ( %d , %d ) size grid \n",Dims[0],Dims[1]);

	MPI_Barrier(MPI_COMM_WORLD);

  	if (gridRank == 0)
  	{
  		/*
  		for (int i = 0; i < Dims[0]; ++i)
  		{
  			for (int j = 0; j < Dims[1]; ++j)
  			{
  				int id;
  				int coord[] = {i,j};
  				MPI_Cart_rank(grid_comm, coord, &id);
  				printf("| %d",id );
  			}
  			printf("\n");
  		}
  		*/
  	}
  	MPI_Barrier(MPI_COMM_WORLD);

  	// Communication via les lignes, on broadcast la diagonal

  	int A = -1;

  	if (gridRank==0)
		{
			A = 1;
			printf("Init value of A for rank %d = %d \n", gridRank, A);
		}
	MPI_Barrier(MPI_COMM_WORLD);
  	if (gridRank==4) 
		{
			A = 2;
			printf("Init value of A for rank %d = %d \n", gridRank, A);
		}
	MPI_Barrier(MPI_COMM_WORLD);
	if (gridRank==8) 
		{
			A = 3;
			printf("Init value of A for rank %d = %d \n", gridRank, A);
		}
	MPI_Barrier(MPI_COMM_WORLD);
  	printf("Row: %d / my_rank : %d / lrank : %d / rrank : %d \n", row, gridRank, lRank, rRank);
  	MPI_Bcast(&A,1,MPI_INT,rRank,line_comm);
  	
  	
  	
	MPI_Barrier(MPI_COMM_WORLD);

	printf("Rank %d value of %d from process %d\n", gridRank, A, ROOT);


	  	//printf("Rank %d received %d\n", my_rank, A);
		




	MPI_Finalize(); // cloturer l'environement MPI
	
 }
