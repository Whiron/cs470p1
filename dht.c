/**
 * dht.c
 *
 * CS 470 Project 4
 *
 * Implementation for distributed hash table (DHT).
 *
 * Name: Georgia Corey & Justin Mikesell
 *
 */

#include <mpi.h>
#include <pthread.h>

#include "dht.h"

/*
 * Private module variable: current process ID
 */
static int pid;
int thread_count;
pthread_t *thread_handle;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* server(void* rank)
{
    return NULL;
}
int dht_init()
{
    local_init();
    int provided;
    MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);
    if (provided != MPI_THREAD_MULTIPLE) {
        printf("ERROR: Cannot initialize MPI in THREAD_MULTIPLE mode.\n");
        exit(EXIT_FAILURE);
    }
    thread_handle = (pthread_t *)malloc(sizeof(pthread_t)*provided);
    for(int i = 0; i < provided; i++){
        pthread_create(&thread_handle[i], NULL, server, (void*)0);
    }
    thread_count = provided;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    printf("%d", pid);
    return pid;
}

void dht_put(const char *key, long value)
{
    pthread_mutex_lock(&mutex);
    local_put(key, value);
    pthread_mutex_unlock(&mutex);
}

long dht_get(const char *key)
{
    return local_get(key);
}

size_t dht_size()
{
    return local_size();
}

void dht_sync()
{
    // nothing to do in the serial version
}

void dht_destroy(FILE *output)
{
    local_destroy(output);
    MPI_Finalize();
    for(int i = 0; i < thread_count; i++){
        pthread_join(thread_handle[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    free(thread_handle);
}

