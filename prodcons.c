/*
 *  prodcons module
 *  Producer Consumer module
 *
 *  Implements routines for the producer consumer module based on
 *  chapter 30, section 2 of Operating Systems: Three Easy Pieces
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 */

// Include only libraries for this module
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"


// Define Locks and Condition variables here
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER; 
pthread_cond_t full  = PTHREAD_COND_INITIALIZER;

// Producer consumer data structures
Matrix * temp;
int fill = 0; 
int use = 0;  
int count = 0; 

// Bounded buffer put() get()
int put(Matrix * value)
{
  bigmatrix[fill] = value;
  fill = (fill + 1) % BOUNDED_BUFFER_SIZE;
  count++;
  return fill;
}

Matrix * get() 
{
  temp = bigmatrix[use];
  use = (use + 1) % BOUNDED_BUFFER_SIZE;
  count--;
  return temp;
}


// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
  Matrix *m = NULL;
  ProdConsStats *ProdStats = (ProdConsStats *) arg;
  for (int i = 0; i < NUMBER_OF_MATRICES; i++) {
    m = GenMatrixRandom();
    pthread_mutex_lock(&mutex);
    while (count == BOUNDED_BUFFER_SIZE) {
      pthread_cond_wait(&empty, &mutex);
    }
    put(m);
    ProdStats->matrixtotal++;
    ProdStats->sumtotal += SumMatrix(m);
    pthread_cond_signal(&full);
    pthread_mutex_unlock(&mutex);
  } 			
  return NULL;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  Matrix *m1 = NULL, *m2 = NULL, *m3 = NULL;
  ProdConsStats *ConsStats = (ProdConsStats *) arg;
  for (int i = 0; i < NUMBER_OF_MATRICES; i++){
    pthread_mutex_lock(&mutex);
    while (count == 0){
      pthread_cond_wait(&full, &mutex);
    }
    if (m1 == NULL) {
      m1 = get();
      ConsStats->matrixtotal++;
      ConsStats->sumtotal += SumMatrix(m1);
    } else if (m2 == NULL) {
      m2 = get();
      ConsStats->matrixtotal++;
      ConsStats->sumtotal += SumMatrix(m2);
      m3 = MatrixMultiply(m1, m2);
      if (m3 == NULL){
        FreeMatrix(m2);  
      	m2 =NULL;
      } else {
        DisplayMatrix(m1, stdout);
        printf("    X\n");
        DisplayMatrix(m2,stdout);
        printf("    =\n");
        DisplayMatrix(m3,stdout);
        printf("\n");
        FreeMatrix(m3);
        FreeMatrix(m2);
        FreeMatrix(m1);
        m1 =NULL;
        m2 =NULL;
        m3 =NULL;
        ConsStats->multtotal++;
      }
    }
    pthread_cond_signal(&empty); // signal
    pthread_mutex_unlock(&mutex); // unlock
  }
  return NULL;
}

