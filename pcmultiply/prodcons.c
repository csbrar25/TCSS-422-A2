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
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t prod_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cons_cond = PTHREAD_COND_INITIALIZER;
counters_t counters_strut;
counter_t buffer_count;
counter_t prod_count, cons_count;

// Bounded buffer put() get()
int put(Matrix * value)
{
	bigmatrix[BOUNDED_BUFFER_SIZE - get_cnt(counters_strut.prod) + get_cnt(counters_strut.cons) - 1] = value;
	increment_cnt(counters_strut.prod);
	return 0;
}

Matrix * get()
{
	increment_cnt(counters_strut.cons);
	return bigmatrix[BOUNDED_BUFFER_SIZE - get_cnt(counters_strut.prod) + get_cnt(counters_strut.cons) - 1];
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
	Matrix *m1 = NULL;
	ProdConsStats *prod_stats = (ProdConsStats *) arg;
	prod_stats->sumtotal = 0;
	prod_stats->matrixtotal = 0;

	pthread_mutex_lock(&mutex);
	if (get_cnt(&buffer_count) == 0 || !get_cnt(&buffer_count)) {
		init_cnt(&buffer_count);
		increment_cnt(&buffer_count);
		counters_strut.prod = &prod_count;
		counters_strut.cons = &cons_count;
		init_cnt(counters_strut.prod);
		init_cnt(counters_strut.cons);
	}
	pthread_mutex_unlock(&mutex);

	while (get_cnt(counters_strut.prod) < NUMBER_OF_MATRICES) {
		pthread_mutex_lock(&mutex);
		while ((get_cnt(counters_strut.prod) - get_cnt(counters_strut.cons)) >= BOUNDED_BUFFER_SIZE && get_cnt(counters_strut.prod) < NUMBER_OF_MATRICES) {
			pthread_cond_wait(&prod_cond, &mutex);
		}
		if (get_cnt(counters_strut.prod) < NUMBER_OF_MATRICES) {
			m1 = GenMatrixRandom();
			put(m1);
			prod_stats->sumtotal += SumMatrix(m1);
			prod_stats->matrixtotal++;
			pthread_cond_signal(&cons_cond);
		}
		pthread_mutex_unlock(&mutex);
	}
	pthread_cond_broadcast(&prod_cond);
	pthread_cond_broadcast(&cons_cond);
	pthread_exit(&prod_stats);
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
	ProdConsStats *cons_stats = (ProdConsStats *) arg;
	cons_stats->matrixtotal = 0;
	cons_stats->sumtotal = 0;
	cons_stats->multtotal = 0;

	pthread_mutex_lock(&mutex);
	if (get_cnt(&buffer_count) == 0 || !get_cnt(&buffer_count)) {
		init_cnt(&buffer_count);
		increment_cnt(&buffer_count);
		counters_strut.prod = &prod_count;
		counters_strut.cons = &cons_count;
		init_cnt(counters_strut.prod);
		init_cnt(counters_strut.cons);
	}
	pthread_mutex_unlock(&mutex);

	while (get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
		Matrix *m1 = NULL, *m2 = NULL, *m3 = NULL;
		pthread_mutex_lock(&mutex);
		while ( (get_cnt(counters_strut.prod) - get_cnt(counters_strut.cons)) <= 0  && get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
			pthread_cond_wait(&cons_cond, &mutex);
		}

		if (get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
			m1 = get();
			cons_stats->sumtotal += SumMatrix(m1);
			cons_stats->matrixtotal++;
			bigmatrix[BOUNDED_BUFFER_SIZE - get_cnt(counters_strut.prod) + get_cnt(counters_strut.cons) - 1] = NULL;
			pthread_cond_signal(&prod_cond);
		}

		if (get_cnt(counters_strut.cons) >= NUMBER_OF_MATRICES) {
			if (m1) FreeMatrix(m1);
			pthread_mutex_unlock(&mutex);
			break;
		}

		while ( (get_cnt(counters_strut.prod) - get_cnt(counters_strut.cons)) <= 0  && get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
			pthread_cond_wait(&cons_cond, &mutex);
		}

		if (get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
			m2 = get();
			cons_stats->sumtotal += SumMatrix(m2);
			cons_stats->matrixtotal++;
			bigmatrix[BOUNDED_BUFFER_SIZE - get_cnt(counters_strut.prod) + get_cnt(counters_strut.cons) - 1] = NULL;
			pthread_cond_signal(&prod_cond);
		}
		pthread_mutex_unlock(&mutex);

		if (m1 && m2) m3 = MatrixMultiply(m1, m2);

		while (!m3 && get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
			if (m2) {
				FreeMatrix(m2);
				m2 = NULL;
			}
			pthread_mutex_lock(&mutex);
			while ( (get_cnt(counters_strut.prod) - get_cnt(counters_strut.cons)) <= 0 && get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
				pthread_cond_wait(&cons_cond, &mutex);
			}
			if (get_cnt(counters_strut.cons) < NUMBER_OF_MATRICES) {
				m2 = get();
				cons_stats->sumtotal += SumMatrix(m2);
				cons_stats->matrixtotal++;
				bigmatrix[BOUNDED_BUFFER_SIZE - get_cnt(counters_strut.prod) + get_cnt(counters_strut.cons) - 1] = NULL;
				pthread_cond_signal(&prod_cond);
			}
			if (m2) {
				m3 = MatrixMultiply(m1, m2);
			}
			pthread_mutex_unlock(&mutex);
		}


		if (m3 != NULL) {
			pthread_mutex_lock(&mutex);
			DisplayMatrix(m1,stdout);
			printf("    X\n");
			DisplayMatrix(m2,stdout);
			printf("    =\n");
			DisplayMatrix(m3,stdout);
			printf("\n");
			cons_stats->multtotal++;
			FreeMatrix(m3);
			pthread_mutex_unlock(&mutex);
		}

		if (m1) FreeMatrix(m1);
		if (m2) FreeMatrix(m2);

	}

	pthread_cond_broadcast(&prod_cond);
	pthread_cond_broadcast(&cons_cond);
	pthread_exit(&cons_stats);
}
