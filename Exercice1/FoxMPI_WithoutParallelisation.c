#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <math.h>
#include <time.h>
#include "Matrice.h"

//Permet le chronométrage
clock_t timer_start;
clock_t timer;

//-------------------------------------------------
//Fonction de chronométrage
double getTime(){
	timer = clock();
	return (double)(timer-timer_start)/CLOCKS_PER_SEC;
}

//-----------------------------------------------------
int main(int argc, char *argv[]){
	int k=0;
	timer_start = clock(); //Démarrage du chrono

	//INITIALISATION
    Matrix A = createMatrix(argv[1]);
	Matrix B = createMatrix(argv[2]);
	Matrix C;
    C.Dim = A.Dim;
	double time_init = getTime();

	//ALGORITHME CLASSIQUE
    C = multiplyMatrix(A, B);
	double time_compute = getTime();

    printf("---------------------------------------\n");
    printf("Valeur de A :\n");
	printMatrix(A);
	printf("Valeur de B :\n");
	printMatrix(B);
	printf("Valeur de C :\n");
	printMatrix(C);
	printf("---------------------------------------\n");
	printf("TEMPS MESURE : \n");
	printf("Temps d'initialisation :    %fsec\n", time_init);
	printf("Temps de calcul :           %fsec\n", time_compute-time_init);
	printf("TEMPS TOTAL :               %fsec\n", time_compute);
    printf("---------------------------------------\n");
    /*printf("Test sur grandes matrices :\n");
    Matrix X = generateMatrix(50);
	printMatrix(X);*/

    

	return 0;

}
