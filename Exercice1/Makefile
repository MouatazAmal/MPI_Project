
all : FoxMPI FoxMPI_WithoutParallelisation

FoxMPI : FoxMPI.o Matrice.o
	mpicc FoxMPI.o Matrice.o -o FoxMPI -lm

FoxMPI.o: FoxMPI.c Matrice.h
	mpicc -c FoxMPI.c -o FoxMPI.o -lm

FoxMPI_WithoutParallelisation : FoxMPI_WithoutParallelisation.o Matrice.o
	gcc FoxMPI_WithoutParallelisation.o Matrice.o -o FoxMPI_WithoutParallelisation

FoxMPI_WithoutParallelisation.o: FoxMPI_WithoutParallelisation.c Matrice.h
	gcc -c FoxMPI_WithoutParallelisation.c -o FoxMPI_WithoutParallelisation.o 

Matrice.o: Matrice.c Matrice.h
	gcc -c Matrice.c -o Matrice.o

clean:
	rm -f *.o