
all : FoxMPI

FoxMPI : FoxMPI.o Matrice.o
	mpicc FoxMPI.o Matrice.o -o FoxMPI -lm

FoxMPI.o: FoxMPI.c Matrice.h
	mpicc -c FoxMPI.c -o FoxMPI.o -lm

Matrice.o: Matrice.c Matrice.h
	gcc -c Matrice.c -o Matrice.o -lm

clean:
	rm -f *.o