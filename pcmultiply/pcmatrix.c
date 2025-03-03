/******************************************************************************
  @file         pcmatrix.c
  @author       Jovany Cardoza-Aguilar, Charankamal Brar
*******************************************************************************/
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

  // Prints arguments used by program
  printf("Producing %d matrices in mode %d.\n",NUMBER_OF_MATRICES,MATRIX_MODE);
  printf("Using a shared buffer of size=%d\n", BOUNDED_BUFFER_SIZE);
  printf("With %d producer and consumer thread(s).\n",numw);
  printf("\n");

  // Here is an example to define one producer and one consumer
  // [numw] creates an array of producer and consumers based on arguments
  pthread_t pr[numw];
  pthread_t co[numw];

  // Add your code here to create threads and so on
  // Bounded Buffer that stores matricies
  bigmatrix = (Matrix **) malloc(sizeof(Matrix *) * BOUNDED_BUFFER_SIZE);

  // ProdConsStats struct that tracks stats of threads
  ProdConsStats prod_stats[numw];
  ProdConsStats cons_stats[numw];

  // Creates threads based on numw, from array of pthreads producers and consumers
  for (int i = 0; i < numw; i++) {
	  pthread_create(&pr[i], NULL, prod_worker, &prod_stats[i]); 
	  pthread_create(&co[i], NULL, cons_worker, &cons_stats[i]); 
  }

  // These are used to aggregate total numbers for main thread output
  int prs = 0; // total #matrices produced
  int cos = 0; // total #matrices consumed
  int prodtot = 0; // total sum of elements for matrices produced
  int constot = 0; // total sum of elements for matrices consumed
  int consmul = 0; // total # multiplications

  // consume ProdConsStats from producer and consumer threads [HINT: return from join]
  // add up total matrix stats in prs, cos, prodtot, constot, consmul
  for (int i = 0; i < numw; i++) {
	  // Wait for threads to finish
	  pthread_join(pr[i], NULL);
	  pthread_join(co[i], NULL);
	  // Sum of matricies
	  prs += prod_stats[i].sumtotal;
	  cos += cons_stats[i].sumtotal;
	  // Number of matricies
	  prodtot += prod_stats[i].matrixtotal;
	  constot += cons_stats[i].matrixtotal;
	  // Number of multiplications
	  consmul += cons_stats[i].multtotal;
  }

  // Prints matrix/thread stats
  printf("Sum of Matrix elements --> Produced=%d = Consumed=%d\n",prs,cos);
  printf("Matrices produced=%d consumed=%d multiplied=%d\n",prodtot,constot,consmul);

  // Frees bigmatrix bounded buffer
  free(bigmatrix);

  return 0;
}
