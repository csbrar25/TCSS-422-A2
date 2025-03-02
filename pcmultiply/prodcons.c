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


// Define Locks, Condition variables, and so on here
extern Matrix **bigmatrix;
int buffer_head = 0;  //Points to next available slot for a consumer
int buffer_tail= 0; // Points to the next available slot for producer
counter_t buffer_count;

// Global Synchronizzation vaiables
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t buffer_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_not_empty = PTHREAD_COND_INITIALIZER;


// Bounded buffer put() get()
int put(Matrix * value)
{
  pthread_mutex_lock(&buffer_mutex);  //Locks the buffer

  //Wait if buffer is full
  while(get_cnt(&buffer_count) == BOUNDED_BUFFER_SIZE) {
    pthread_cond_wait(&buffer_not_full, &buffer_mutex);
  }

  bigmatrix[buffer_tail] = value; //Add matrix at buffer tail and move buffer tail forward.
  buffer_tail = (buffer_tail + 1)% BOUNDED_BUFFER_SIZE; //Wrap around if buffer tail reaches end.
  increment_cnt(&buffer_count); //Increment buffer count

  pthread_cond_signal(&buffer_not_empty); //Signals consumers that a new item is available
  pthread_mutex_unlock(&buffer_mutex);  //Unlocks the buffer

  return 0;
}

Matrix * get()
{
  pthread_mutex_lock(&buffer_mutex);  //Locks the buffer

  //Wait if buffer is empty
  while(get_cnt(&buffer_count) == 0) {
    pthread_cond_wait(&buffer_not_empty, &buffer_mutex);
  }
  
  Matrix *mat = bigmatrix[buffer_head]; //Get matrix from buffer at buffer head
  buffer_head = (buffer_head + 1) % BOUNDED_BUFFER_SIZE;  //Wrap around if buffer head reaches the end
  decrement_cnt(&buffer_count); //Decrement buffer count

  pthread_cond_signal(&buffer_not_full);  // Signal producers the space is now available.
  pthread_mutex_unlock(&buffer_mutex);  //Unlock the buffer

  return mat; //returns the retrieved matrix. 
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
  int *produced_count = malloc (sizeof(int)); //Track the number of matrices produced
  *produced_count = 0;

  for (int i = 0; i < NUMBER_OF_MATRICES; i++) {
    Matrix *m = GenMatrixRandom();  //Generate a random matrix
    put(m); //place the matrix in the buffer
    (*produced_count)++;  //Track produced matrices
  }

  for (int i = 0; i < NUMWORK; i++) {
        put(NULL);  // Insert NULL into the buffer as a termination signal
  }

  pthread_exit((void *)produced_count); //Return the count
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  int *consumed_count = malloc(sizeof(int));
  *consumed_count  = 0;

  while(1) {
    Matrix *m1 = get(); //get first matrix from buffer
    if(m1 == NULL)  break;

    Matrix *m2;
    do {
      m2 = get(); //get second matrix
      if(m2 == NULL) {
        FreeMatrix(m1);
        break;
      }
    } while (m1 -> cols != m2-> rows);  //Repeat untill a valid pair is found

    if (m2 == NULL) continue; //If no vaild second matirx found, continue

    Matrix *result = MatrixMultiply(m1, m2);

    pthread_mutex_lock(&print_mutex);
    printf("\nMULTIPLY (%d x %d) BY (%d x %d):\n", m1->rows, m1->cols, m2->rows, m2->cols);

    if(result != NULL) {  //if result is not  empty print the result matrix
      printf("Matrix Multiplication Result:\n");
      DisplayMatrix(result,stdout);
      FreeMatrix(result);
    }
    pthread_mutex_unlock(&print_mutex);

    FreeMatrix(m1);
    FreeMatrix(m2);
    (*consumed_count)++;
  }

  pthread_exit((void *)consumed_count); //return the count
}
