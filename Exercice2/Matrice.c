#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Matrice.h"

//-----------------------------------------------------
//Va créer une matrice à partir d'un fichier (cf matrice1.txt)
Matrix createMatrix(char* filename){
    char ch;
    FILE *fp;
    fp = fopen(filename, "r"); 
 
    if (fp == NULL){
      perror("Erreur : Echec d'ouverture du fichier.\n");
      exit(EXIT_FAILURE);
    }

    Matrix M;

    if((ch = fgetc(fp)) != EOF){
        M.Dim = ch  - '0'; //La première ligne du fichier donne la dimension de la matrice
        int i=0,j=0;
        ch = fgetc(fp);

        while((ch = fgetc(fp)) != EOF){
            
            if (ch == '\n'){
                i++;
                j=0;
            }
            if ((ch >= '0')&&(ch <= '9')){
                M.M[i][j] = ch  - '0';
                j++;
            }
            
        }
        fclose(fp);
        return M;
    
    }
    else{
        return M;
    }
 
   fclose(fp);
   return M;
}
//----------------------------------------------------
//Affichage de la matrice M
void printMatrix(Matrix M){
    
    int i,j;

    for(i=0; i<M.Dim;i++){
        for(j=0; j<M.Dim;j++){
            printf("%d ", M.M[i][j]);
        }
        printf("\n");
    }
    
   return;
}
//----------------------------------------------------