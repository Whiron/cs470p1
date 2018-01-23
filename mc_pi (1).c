/**
 * mc_pi.c
 *
 * CS 470 Pthreads Exercise 1
 * Based on IPP Programming Assignment 4.2
 *
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

int thread_count;               // global thread count
long total_darts = 0;           // global dart count

long darts_in_circle = 0;       // global dart tracker

static unsigned long seed = 0;  // random number generator seed
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

double rand_dbl()
{
    // basic linear congruential generator (LCG) algorithm
    // (see https://en.wikipedia.org/wiki/Linear_congruential_generator)
    //
    seed = (1103515245*seed + 12345) % (1<<31);
    return ((double)seed / ULONG_MAX)*2.0 - 1.0;
}

void* throw_darts(void* rank)
{
    long start = 0;
    long stop = total_darts / thread_count;
    seed = (unsigned long)rank;
    int updates = 0;

    for (long dart = start; dart < stop; dart++) {

        // throw a dart
        double x = rand_dbl();
        double y = rand_dbl();
        double dist_sq = x*x + y*y;

        // update global dart tracker
        if (dist_sq <= 1.0) {
             updates++;
        }
    }

    pthread_mutex_lock(&mutex);
    darts_in_circle += updates;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char* argv[])
{
    struct timeval tv;
    double start, end;
    srand((unsigned)time(NULL));

    // check and parse command-line arguments
    if (argc != 3) {
        printf("Usage: %s <num-threads> <num-darts>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    thread_count = strtol(argv[1], NULL, 10);
    total_darts = strtoll(argv[2], NULL, 10);

    // initialize threading
    
    pthread_t *thread_handle;
    thread_handle = (pthread_t*)malloc(sizeof(pthread_t)*thread_count);

    gettimeofday(&tv, NULL);
    start = tv.tv_sec+(tv.tv_usec/1000000.0);

    for (int i = 0; i < thread_count; i++)
    {
        pthread_create(&thread_handle[i], NULL, throw_darts, (void*)0);   
    }

    for (int i = 0; i < thread_count; i++)
    {
        pthread_join(thread_handle[i], NULL);
    }

    gettimeofday(&tv, NULL);
    end = tv.tv_sec+(tv.tv_usec/1000000.0);
    
    // calculate pi
    double pi_est = 4 * darts_in_circle / ((double)total_darts);
    printf("Estimated pi: %e   Time elapsed: %.3lfs  w/  %d thread(s)\n",
            pi_est, end-start, thread_count);

    // clean up
    pthread_mutex_destroy(&mutex);
    free(thread_handle);
    return EXIT_SUCCESS;
}


