#include <stdio.h>
#include <string.h>
#include <mpi.h>
#define N 10
#define ROOT 0

// Compilation : mpicc -o Exercice1 Exercice1.c

// Exécution : mpirun -n 100 ./Exercice1
// Le 2 indique le nombre de processus à faire tourner
// On indique ensuite sur quel machines on veut faire tourner les processus [facultatif !]

int main(int argc, char *argv[]){
		
	int my_rank,col,row;
	MPI_Status status;
	MPI_Comm grid_comm,cc,cl;
	int gridRank;
	int coordinates[2];
	int period[2] = {1,1};

	int A[N][N];
	int B[N][N];
	int C[N][N];

	MPI_Init(&argc, &argv); // Initialise l'env d'exécution
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	int nbProcess;
	MPI_Comm_size(MPI_COMM_WORLD,&nbProcess);
	int Dims[2] = {0,0};
	MPI_Dims_create(nbProcess,2,Dims);
 	MPI_Cart_create(MPI_COMM_WORLD, 2, Dims, period, 1,&grid_comm);

 	MPI_Comm_rank(grid_comm,&gridRank);
    MPI_Cart_coords(grid_comm, gridRank, 2, coordinates);
    row = coordinates[0];
    col = coordinates[1];

    int remain[2] = {1,0};
	MPI_Cart_sub(grid_comm,remain, &cl);
	remain[0] = 0;
	remain[1] = 1;
	MPI_Cart_sub(grid_comm,remain, &cc);
	if (my_rank == ROOT) printf("The created Grid is ( %d , %d ) size grid \n",Dims[0],Dims[1]);
	MPI_Barrier(MPI_COMM_WORLD);
	int id1,id2;
	
    int coord[] = {3,1};
    MPI_Cart_rank(grid_comm, coord, &id1);
    coord[0] = 3;
    coord[1] = 2;
    MPI_Cart_rank(grid_comm, coord, &id2);

    int buff = -1;
    if (my_rank == ROOT)
    {
    	buff = 42;
    }
    MPI_Bcast(&buff, 1, MPI_INT, ROOT, cl);

    if (my_rank != ROOT) {
	    printf("Rank %d receives %d from process %d\n", my_rank, buff, ROOT);
	}
    
    /*
    if (my_rank == id1)
    {
    	int number = 42;
    	MPI_Send(&number, 1, MPI_INT, id2,99, cl);
    	printf("rank 31 send %d\n",number );
    	MPI_Recv(&number,1,MPI_INT,id2,99,grid_comm,&status);
    	printf("I received %d\n",number);
    }
     if (my_rank == id2)
    {
    	int Data;
    	MPI_Recv(&Data,1,MPI_INT,id1,99,cl,&status);
    	printf("I received %d\n",Data);
    	Data = Data + 1;
    	MPI_Send(&Data, 1, MPI_INT, id1,99, grid_comm);
    }
    */

	MPI_Finalize(); // cloturer l'environement MPI
	
 }