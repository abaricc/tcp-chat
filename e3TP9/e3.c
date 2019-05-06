#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_THREADS 100

typedef struct task_params {
  int row;
  int column;
} task_params;

task_params params[27];
pthread_t th[27];

void creerTaches() {
  for(int i=0; i<9; i++) {//lignes
    params[i].row=i+1;
    params[i].column=0;
  }
  for(int j=0; j<9; j++) {//colonnes
    params[9+j].row=0;
    params[9+j].column=j;
  }
  for(int k=0; k<9; k++) {
    params[18+k].row=(k%3)+1;//3*(k%3)+1
    params[18+k].column=(k/3)+1;//3*(k/3)+1
  }
}

void *tmain(void *arg) {
  int i= (int) arg;
  printf("je suis le thread %d mes arguments sont ligne=%d et colonne=%d\n", i, params[i].row, params[i].column);
  if(params[i].column==0)
    res[i]=testerLigne(params[i].row-1);
  else if (params[i].row==0)
    res[i]=testerLigne(params[i].column-1);
  else
    res[i]=testerCarre(params[i].row-1, params[i].column-1);
}

int main(int argc, char *argv[]) {
  creerTaches();
  for(int k=0; k<27; k++) {
    //creer thread en passant i comme argument
    pthread_create(&th[k], NULL, tmain, (void*)k);
  }
  //attend les threads
  for(int k=0; k<27; k++) {
    pthread_join(th[k], NULL);
    //afficher resultat
  }
}
