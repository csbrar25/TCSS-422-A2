/******************************************************************************
  @file         prodcons.c
  @author       Jovany Cardoza-Aguilar, Charankamal Brar
*******************************************************************************/
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
// Mutex that is used for locking and unlocking critical sections of code, 
// to stop multiple thread from accessing critical sections at the same time
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// Conds that are used to coordinate threads
pthread_cond_t prod_cond = PTHREAD_COND_INITIALIZER;	// Used to signal producer to resume producing matrices
pthread_cond_t cons_cond = PTHREAD_COND_INITIALIZER;	// Used to signal consumers to resume consuming matrices
counters_t counters_strut;	// Counters struct for production and consumer counters
counter_t buffer_count;		// Counter, that tracks buffer
counter_t prod_count, cons_count;	// Counters for production and consumer 

// Bounded buffer put() get()
// Puts a matrix into bounded buffer
int put(Matrix * value) 
{
	// Put matrix into bigmatrix bounded buffer, 
	// calculates location based on size of buffer and minus it by number of matrices produced
	// and adds by number of matrices consumed and minuses by 1, to determine location to put matrix 
	bigmatrix[BOUNDED_BUFFER_SIZE - get_cnt(counters_strut.prod) + get_cnt(counters_strut.cons) - 1] = value;
	// Increment producer counter
	increment_cnt(counters_strut.prod);
	return 1;
}

// Get a matrix from bounded buffer
Matrix * get()
{
	// Increment consumer counter
	increment_cnt(counters_strut.cons);
	// Retrieves matrix from bounded buffer,
	// calculates location based on size of buffer and minus it by number of matrices produced
	// and adds by number of matrices consumed and minuses by 1, to determine location to put matrix 
	return bigmatrix[BOUNDED_BUFFER_SIZE - get_cnt(counters_strut.prod) + get_cnt(counters_strut.cons) - 1];
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
	// Matrix to be added to bounded buffer
	Matrix *m1 = NULL;
	// Initializes producer stats for thread
	ProdConsStats *prod_stats = (ProdConsStats *) arg;
	// Sets producer stats to 0
	prod_stats->sumtotal = 0;
	prod_stats->matrixtotal = 0;

	pthread_mutex_lock(&mutex);
	// Initializes counters, if they haven't been initialized yet
	if (get_cnt(&buffer_count) == 0 || !get_cnt(&buffer_count)) {
		// Initializes and increments buffer_count, to only initialize once
		init_cnt(&buffer_count);
		increment_cnt(&buffer_count);
		// Initializes producer and consumer counters
		counters_strut.prod = &prod_count;
		counters_strut.cons = &cons_count;
		init_cnt(counters_strut.prod);
		init_cnt(counters_strut.cons);
	}
	pthread_mutex_unlock(&mutex);

	// Loops until expected number of matrices are produced
	while (get_cnt(counters_strut.prod) < NUMBER_OF_MATRICES) {
		pthread_mutex_lock(&mutex);
		// Skips if there is no space in the bounded buffer or expected number of matrices has been produced
		while ((get_cnt(counters_strut.prod) - get_cnt(counters_strut.cons)) >= BOUNDED_BUFFER_SIZE && get_cnt(counters_strut.prod) < NUMBER_OF_MATRICES) {
			// Waits until signal is received that indicates space in the bounded buffer
			pthread_cond_wait(&prod_cond, &mutex);
		}
		// Checks that the number of produced matrices is less than expected number of matrices
		if (get_cnt(counters_strut.prod) < NUMBER_OF_MATRICES) {
			// Generates matrix
			m1 = GenMatrixRandom();
			// Puts matrix into bounded buffer
			put(m1);
			// Adds sum of matrix to producer stats
			prod_stats->sumtotal += SumMatrix(m1);
			// Increments total matrices produced from producer stats
			prod_stats->matrixtotal++;
			// Signals that matrix has been produced and put into bounded buffer
			pthread_cond_signal(&cons_cond);
		}
		pthread_mutex_unlock(&mutex);
	}

	// Broadcast signals for threads that are still waiting
	pthread_cond_broadcast(&prod_cond);
	pthread_cond_broadcast(&cons_cond);
	pthread_exit(&prod_stats);
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
	// Initializes consumer stats for thread
	ProdConsStats *cons_stats = (ProdConsStats *) arg;
	// Sets consumer stats to 0
	cons_stats->matrixtotal = 0;
	cons_stats->sumtotal = 0;
	cons_stats->multtotal = 0;

	pthread_mutex_lock(&mutex);
	// Initializes counters, if they haven't been initialized yet
	if (get_cnt(&buffer_count) == 0 || !get_cnt(&buffer_count)) {
		// Initializes and increments buffer_count, to only initialize once
		init_cnt(&buffer_count);
		increment_cnt(&buffer_count);
		// Initializes producer and consumer counters
		counters_strut.prod = &prod_count;
		counters_strut.cons = &cons_count;
		init_cnt(counters_strut.prod);
		init_cnt(counters_strut.cons);
	}
	pthread_mutex_unlock(&mutex);

	// Loops until expected number of matrices are consumed
	while (get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
		// Matrices to be consumed
		Matrix *m1 = NULL, *m2 = NULL, *m3 = NULL;
		pthread_mutex_lock(&mutex);
		// Skips if bounded buffer is not empty or expected number of matrices has been consumed
		while ( (get_cnt(counters_strut.prod) - get_cnt(counters_strut.cons)) <= 0  && get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
			// Waits until signal is received that indicates matrix has been added to the bounded buffer
			pthread_cond_wait(&cons_cond, &mutex);
		}

		// Checks that the number of consumed matrices is less than expected number of matrices
		if (get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
			// Gets matrix from bounded buffer
			m1 = get();
			// Adds sum of matrix to consumer stats
			cons_stats->sumtotal += SumMatrix(m1);
			// Increments total matrices consumer from consumer stats
			cons_stats->matrixtotal++;
			// Sets location of matrix from bounded buffer to be NULL
			bigmatrix[BOUNDED_BUFFER_SIZE - get_cnt(counters_strut.prod) + get_cnt(counters_strut.cons) - 1] = NULL;
			// Signals that matrix has been consumed and there is space in the bounded buffer
			pthread_cond_signal(&prod_cond);
		}

		// If number of consumed threads equals expected number of matrices, break and loop and exit thread
		if (get_cnt(counters_strut.cons) >= NUMBER_OF_MATRICES) {
			// Free m1 matrix
			if (m1) FreeMatrix(m1);
			pthread_mutex_unlock(&mutex);
			break;
		}

		// Skips if bounded buffer is not empty or expected number of matrices has been consumed
		while ( (get_cnt(counters_strut.prod) - get_cnt(counters_strut.cons)) <= 0  && get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
			// Waits until signal is received that indicates matrix has been added to the bounded buffer
			pthread_cond_wait(&cons_cond, &mutex);
		}

		// Checks that the number of consumed matrices is less than expected number of matrices
		if (get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
			// Gets matrix from bounded buffer
			m2 = get();
			// Adds sum of matrix to consumer stats
			cons_stats->sumtotal += SumMatrix(m2);
			// Increments total matrices consumer from consumer stats
			cons_stats->matrixtotal++;
			// Sets location of matrix from bounded buffer to be NULL
			bigmatrix[BOUNDED_BUFFER_SIZE - get_cnt(counters_strut.prod) + get_cnt(counters_strut.cons) - 1] = NULL;
			// Signals that matrix has been consumed and there is space in the bounded buffer
			pthread_cond_signal(&prod_cond);
		}
		pthread_mutex_unlock(&mutex);

		// Multiplies matrices, if m1 and m2 are not NULL
		if (m1 && m2) m3 = MatrixMultiply(m1, m2);

		// Loops until MatrixMultiply is successful, or runs out of matrices
		while (!m3 && get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
			// If m2 is not NULL, free matrix and set to NULL to not have memory leak
			if (m2) {
				FreeMatrix(m2);
				m2 = NULL;
			}
			pthread_mutex_lock(&mutex);
			// Skips if bounded buffer is not empty or expected number of matrices has been consumed
			while ( (get_cnt(counters_strut.prod) - get_cnt(counters_strut.cons)) <= 0 && get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
				// Waits until signal is received that indicates matrix has been added to the bounded buffer
				pthread_cond_wait(&cons_cond, &mutex);
			}
			// Checks that the number of consumed matrices is less than expected number of matrices
			if (get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
				// Gets matrix from bounded buffer
				m2 = get();
				// Adds sum of matrix to consumer stats
				cons_stats->sumtotal += SumMatrix(m2);
				// Increments total matrices consumer from consumer stats
				cons_stats->matrixtotal++;
				// Sets location of matrix from bounded buffer to be NULL
				bigmatrix[BOUNDED_BUFFER_SIZE - get_cnt(counters_strut.prod) + get_cnt(counters_strut.cons) - 1] = NULL;
				// Signals that matrix has been consumed and there is space in the bounded buffer
				pthread_cond_signal(&prod_cond);
			}
			// Check that m2 is not empty to multiply m1 and m2
			if (m2) {
				m3 = MatrixMultiply(m1, m2);
			}
			pthread_mutex_unlock(&mutex);
		}

		// If MatrixMultiplcation was successful, print m1, m2, and resulting m3 matrices
		if (m3 != NULL) {
			pthread_mutex_lock(&mutex);
			DisplayMatrix(m1,stdout);
			printf("    X\n");
			DisplayMatrix(m2,stdout);
			printf("    =\n");
			DisplayMatrix(m3,stdout);
			printf("\n");
			// Increments total matrices multiplied from consumer stats
			cons_stats->multtotal++;
			// Free multiplied matrix
			FreeMatrix(m3);
			pthread_mutex_unlock(&mutex);
		}

		// Free m1 and m2 matrices
		if (m1) FreeMatrix(m1);
		if (m2) FreeMatrix(m2);

	}

	// Broadcast signals for threads that are still waiting
	pthread_cond_broadcast(&prod_cond);
	pthread_cond_broadcast(&cons_cond);
	pthread_exit(&cons_stats);
}
