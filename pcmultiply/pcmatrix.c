#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include "matrix.h"
#include "counter.h"
#include "prodcons.h"
#include "pcmatrix.h"

// Define variables
int BOUNDED_BUFFER_SIZE = MAX;
int NUMBER_OF_MATRICES = LOOPS;
int MATRIX_MODE = DEFAULT_MATRIX_MODE;
Matrix **bigmatrix = NULL;

extern counter_t buffer_count;  // External counter for tracking buffer usage

int main(int argc, char *argv[])
{
    int num_producers = NUMWORK;
    int num_consumers = NUMWORK;

    // Process command-line arguments
    if (argc >= 2) num_producers = atoi(argv[1]);
    if (argc >= 3) num_consumers = atoi(argv[2]);
    if (argc >= 4) BOUNDED_BUFFER_SIZE = atoi(argv[3]);
    if (argc >= 5) NUMBER_OF_MATRICES = atoi(argv[4]);
    if (argc >= 6) MATRIX_MODE = atoi(argv[5]);

    printf("CONFIGURATION: producers=%d consumers=%d buffer_size=%d matrices=%d matrix_mode=%d\n",
           num_producers, num_consumers, BOUNDED_BUFFER_SIZE, NUMBER_OF_MATRICES, MATRIX_MODE);

    srand((unsigned) time(NULL)); // Initialize random seed
    init_cnt(&buffer_count);

    // Allocate memory for bounded buffer
    bigmatrix = (Matrix **) malloc(sizeof(Matrix *) * BOUNDED_BUFFER_SIZE);
    if (bigmatrix == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory for bigmatrix.\n");
        exit(EXIT_FAILURE);
    }

    // Create producer and consumer thread arrays
    pthread_t producers[num_producers], consumers[num_consumers];
    int *produced_counts[num_producers], *consumed_counts[num_consumers];

    // Launch producer threads
    for (int i = 0; i < num_producers; i++) {
        pthread_create(&producers[i], NULL, prod_worker, NULL);
    }

    // Launch consumer threads
    for (int i = 0; i < num_consumers; i++) {
        pthread_create(&consumers[i], NULL, cons_worker, NULL);
    }

    // Wait for all producers to finish
    for (int i = 0; i < num_producers; i++) {
        pthread_join(producers[i], (void **) &produced_counts[i]);
    }

    // Wait for all consumers to finish
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(consumers[i], (void **) &consumed_counts[i]);
    }

    // Aggregate results
    int total_produced = 0, total_consumed = 0;
    for (int i = 0; i < num_producers; i++) total_produced += *produced_counts[i];
    for (int i = 0; i < num_consumers; i++) total_consumed += *consumed_counts[i];

    // Print final statistics
    printf("\nFinal Report:\n");
    printf("Total Matrices Produced: %d\n", total_produced);
    printf("Total Matrices Consumed: %d\n", total_consumed);

    // Cleanup
    for (int i = 0; i < num_producers; i++) free(produced_counts[i]);
    for (int i = 0; i < num_consumers; i++) free(consumed_counts[i]);
    free(bigmatrix);

    return 0;
}
