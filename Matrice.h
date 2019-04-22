#ifndef MATRIX
#define MATRIX

#include <stdio.h>
#include <string.h>
#define MAXSIZE 100

//---------------------------------------------------
//Définition d'une simple matrice carrée qxq
typedef struct{
    int Dim; //Dimension de la matrice
    int M[MAXSIZE][MAXSIZE]; //La fameuse matrice
}Matrix;

//---------------------------------------------------
Matrix createMatrix(char* filename);
void printMatrix(Matrix M);
Matrix addMatrix(Matrix X, Matrix Y);
void initMatrix(Matrix A);
Matrix multiplyMatrix(Matrix A, Matrix B);

//---------------------------------------------------
#endif