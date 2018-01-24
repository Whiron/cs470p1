/*
 * par_sum.c
 *
 * CS 470 Project 1 (Pthreads)
 * Parallel version
 * Date: 1/26/18
 * Authors: Georgia Corey, Justin Mikesell
 * Compile with --std=c99
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

// aggregate variables
long sum = 0;
long odd = 0;
long min = INT_MAX;
long max = INT_MIN;
bool done = false;
int threads = 0;

// Initialize list
struct node
{
    int value;
    struct node *next;
};

typedef struct node linkedList;
linkedList *head_node, *first_node, *temp_node = 0, *prev_node, next_node;
int data;
int count;

// Initialize mutex
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


// function prototypes
void update(long number);
void* handle();
void add(long num);

/*
 * update global aggregate variables given a number
 *
*/
void update(long number)
{
    // simulate computation
    sleep(number);
    
    // update aggregate variables    
    sum += number;

    if (number % 2 == 1) 
    {  
        odd++;
    }
    if (number < min) 
    {
        min = number;
    }
    if (number > max) 
    {  
        max = number;
        
    }
    return;    
}

/*
 * Processes threads to run the update function
 * moving through the list of values.
*/
void* handle()
{
  // Copies count value into local variable.
  int pcount = (uintptr_t)count;
  long val;

  // Go through the list until all required elements are done.
  while (pcount > 0)
  {
 
    pthread_mutex_lock(&mutex);
    val = temp_node->value;     
        
    temp_node = temp_node -> next;
    pthread_mutex_unlock(&mutex);
    update(val);
    pcount--; 
    
    // Prevent possible seg faults.
    if(temp_node == NULL)
    {
        return NULL;
    }    
  }
    return NULL; 
  
}

int main(int argc, char* argv[])
{
    // check and parse command line options
    if (argc != 3) 
    {
        printf("Usage: sum <infile> <number of threads>\n");
        exit(EXIT_FAILURE);
    }
    char *fn = argv[1];
    threads = strtol(argv[2], NULL, 10);
    
    if (threads <=0)
    {
        printf("ERROR: Invalid Thread Count.");
        exit(EXIT_FAILURE);
    }
    
    // Make threads
    pthread_t *thread_handle;
    thread_handle = (pthread_t*)malloc(sizeof(pthread_t)*threads);
    
    // load numbers and add them to the queue
    FILE* fin = fopen(fn, "r");
    char action;
    long num;
    while (fscanf(fin, "%c %ld\n", &action, &num) == 2) 
    {
            
        if (action == 'p') 
        {   // process
            add(num);
            count++;
        } 
        else if (action == 'w') 
        {
            // wait
            sleep(num);
        }
        else 
        {
            printf("ERROR: Unrecognized action: '%c'\n", action);
            exit(EXIT_FAILURE);
        }

    }
    
    // Number of threads should not exceed number of elements in list.
    if(threads > count)
    {
        threads = count;
    }

    // Split up work between threads evenly.
    if(count%threads == 0)
    {
      count = count/threads;
    }
    else
    {
      count = (count/threads)+ 1;      
    }

    temp_node = first_node;
    
    // Make threads
    for(int i = 0; i < threads; i++)
    {
        pthread_create(&thread_handle[i], NULL, handle,(void*)NULL);     
        
    }
    
    // Kill threads
    for (int i = 0; i < threads; i++)
    {
        pthread_join(thread_handle[i], NULL);
    }
        
    fclose(fin);

    // print results
    printf("%ld %ld %ld %ld\n", sum, odd, min, max);

    // clean up and return
    pthread_mutex_destroy(&mutex);
    free(thread_handle);
    return (EXIT_SUCCESS);
}

/*
 * Add elements to linked list.
*/
void add(long num)
{
  
  // Allocates memory for list
  temp_node = (linkedList *) malloc(sizeof (linkedList));

  // Adds value into list
  temp_node->value = num;

  // If the first node is empty, fill it. 
  // Otherwise, go to next position in the list.
  if (first_node == 0) 
  {
    first_node = temp_node;
  } 
  else 
  {
    head_node->next = temp_node;
  }
  // Make the next position in the list empty.
  temp_node->next = 0;
  head_node = temp_node;
}
