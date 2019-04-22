
all : CannonAlgo


CannonAlgo : CannonAlgo.o Matrice.o
	mpicc CannonAlgo.o Matrice.o -o CannonAlgo -lm


CannonAlgo.o: CannonAlgo.c  Matrice.h
	mpicc -c CannonAlgo.c -o CannonAlgo.o -lm

Matrice.o: Matrice.c Matrice.h
	gcc -c Matrice.c -o Matrice.o -lm

clean:
	rm -f *.o