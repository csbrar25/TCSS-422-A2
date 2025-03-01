/*
 *  pcmatrix module
 *  Primary module providing control flow for the pcMatrix program
 *
 *  Producer consumer bounded buffer program to produce random matrices in parallel
 *  and consume them while searching for valid pairs for matrix multiplication.
 *  Matrix multiplication requires the first matrix column count equal the
 *  second matrix row count.
 *
 *  A matrix is consumed from the bounded buffer.  Then matrices are consumed
 *  from the bounded buffer, ONE AT A TIME, until an eligible matrix for multiplication
 *  is found.
 *
 *  Totals are tracked using the ProdConsStats Struct for each thread separately:
 *  - the total number of matrices multiplied (multtotal from each consumer thread)
 *  - the total number of matrices produced (matrixtotal from each producer thread)
 *  - the total number of matrices consumed (matrixtotal from each consumer thread)
 *  - the sum of all elements of all matrices produced and consumed (sumtotal from each producer and consumer thread)
 *  
 *  Then, these values from each thread are aggregated in main thread for output
 *
 *  Correct programs will produce and consume the same number of matrices, and
 *  report the same sum for all matrix elements produced and consumed.
 *
 *  Each thread produces a total sum of the value of
 *  randomly generated elements.  Producer sum and consumer sum must match.
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include "matrix.h"
#include "counter.h"
#include "prodcons.h"
#include "pcmatrix.h"

int BOUNDED_BUFFER_SIZE;
int NUMBER_OF_MATRICES;
int MATRIX_MODE;
Matrix **bigmatrix = NULL; 

extern counter_t buffer_count; 

int main (int argc, char * argv[])
{
  // Process command line arguments
  int numw = NUMWORK;
  if (argc==1)
  {
    BOUNDED_BUFFER_SIZE=MAX;
    NUMBER_OF_MATRICES=LOOPS;
    MATRIX_MODE=DEFAULT_MATRIX_MODE;
    printf("USING DEFAULTS: worker_threads=%d bounded_buffer_size=%d matricies=%d matrix_mode=%d\n",numw,BOUNDED_BUFFER_SIZE,NUMBER_OF_MATRICES,MATRIX_MODE);
  }
  else
  {
    if (argc==2)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=MAX;
      NUMBER_OF_MATRICES=LOOPS;
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==3)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=LOOPS;
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==4)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=atoi(argv[3]);
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==5)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=atoi(argv[3]);
      MATRIX_MODE=atoi(argv[4]);
    }
    printf("USING: worker_threads=%d bounded_buffer_size=%d matricies=%d matrix_mode=%d\n",numw,BOUNDED_BUFFER_SIZE,NUMBER_OF_MATRICES,MATRIX_MODE);
  }

  time_t t;
  // Seed the random number generator with the system time
  srand((unsigned) time(&t));

  // Allocate memory for bounded buffer
    bigmatrix = (Matrix **) malloc(sizeof(Matrix *) * BOUNDED_BUFFER_SIZE);
    if (bigmatrix == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory for bigmatrix.\n");
        exit(EXIT_FAILURE);
    }

  init_cnt(&buffer_count);
  pthread_t producer, consumer;
  int *produced_count, *consumed_count;

  //Create producer and consumer threads
  pthread_create(&producer, NULL, prod_worker, NULL);
  pthread_create(&consumer, NULL, cons_worker, NULL);

  // Wait for threads to finish
    pthread_join(producer, (void **)&produced_count);
    pthread_join(consumer, (void **)&consumed_count);
  
  //Print summary
  printf("Total Matrices Produced: %d\n", *produced_count);
  printf("Total Matrices Consumed: %d\n", *consumed_count);

    // Cleanup
    free(produced_count);
    free(consumed_count);

    return 0;
}
