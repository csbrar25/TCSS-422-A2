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
#define BOUNDED_BUFFER_SIZE 10
Matrix *bigmatrix[BOUNDED_BUFFER_SIZE];
int buffer_head = 0;  //Points to next available slot for a consumer
int buffer_tail= 0; // Points to the next available slot for producer
counter_t buffer_count;

// Global Synchronizzation vaiables
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_not_empty = PTHREAD_COND_INITIALIZER;


// Bounded buffer put() get()
int put(Matrix * value)
{
  pthread_mutex_lock(&buffer_mutex);  //Locks the buffer

  //Wait if buffer is full
  while(buffer_count == BOUNDED_BUFFER_SIZE) {
    pthread_cond_wait(&buffer_not_full, &buffer_mutex);
  }

  bigmatrix[buffer_tail] = value; //Add matrix at buffer tail and move buffer tail forward.
  buffer_tail = (buffer_tail + 1)% BOUNDED_BUFFER_SIZE; //Wrap around if buffer tail reaches end.
  buffer_count++; //Increment buffer count

  pthread_cond_signal(&buffer_not_empty); //Signals consumers that a new item is available
  pthread_mutex_unlock(&buffer_mutex);  //Unlocks the buffer
}

Matrix * get()
{
  pthread_mutex_lock(&buffer_mutex);  //Locks the buffer

  //Wait if buffer is empty
  while(buffer_count == 0) {
    pthread_cond_wait(&buffer_not_empty, &buffer_mutex);
  }
  
  Matrix *mat = bigmatrix[buffer_head]; //Get matrix from buffer at buffer head
  buffer_head = (buffer_head + 1) % BOUNDED_BUFFER_SIZE;  //Wrap around if buffer head reaches the end
  buffer_count--; //Decrement buffer count

  pthread_cond_signal(&buffer_not_full);  // Signal producers the space is now available.
  pthread_mutex_unlock(&buffer_mutex);  //Unlock the buffer

  return mat; //returns the retrieved matrix. 
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
  return NULL;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  return NULL;
}
