/*
 * par_sum.c
 *
 * CS 470 Project 1 (Pthreads)
 * Serial version
 *
 * Compile with --std=c99
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// aggregate variables
long sum = 0;
long odd = 0;
long min = INT_MAX;
long max = INT_MIN;
bool done = false;
static unsigned long value = 0;


struct node
{
    int value;
    struct node *next;
};

typedef struct node linkedList;
linkedList *head_node, *first_node, *temp_node = 0, *prev_node, next_node;
int data;


// function prototypes
void update(void* number);

/*
 * update global aggregate variables given a number
 */
void update(void* number)
{
    value = (unsigned long)number;
    // simulate computation
    sleep(value);

    // update aggregate variables
    sum += value;
    if (value % 2 == 1) 
    {
        odd++;
    }
    if (value < min) 
    {
        min = value;
    }
    if (value > max) 
    {
        max = value;
    }
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
    int threads = strtol(argv[2], NULL, 10);
    
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
        } 
        else if (action == 'w') 
        {
            // wait
            sleep(num);
        }

    }
    display();
    exit(EXIT_SUCCESS);
    for(int i = 0; i < threads; i++)
    {
        pthread_create(&thread_handle[i], NULL, update, (void*)num);
    }
    

    for (int i = 0; i < threads; i++)
    {
        pthread_join(thread_handle[i], NULL);
    }
        /*else 
        {
            printf("ERROR: Unrecognized action: '%c'\n", action);
            exit(EXIT_FAILURE);
        }*/
    
    fclose(fin);

    // print results
    printf("%ld %ld %ld %ld\n", sum, odd, min, max);

    // clean up and return
    return (EXIT_SUCCESS);
}

void add(int num)
{
  temp_node = (linkedList *) malloc(sizeof (linkedList));

  temp_node->value = num;

  if (first_node == 0) 
  {
    first_node = temp_node;
  } 
  else 
  {
    head_node->next = temp_node;
  }
  temp_node->next = 0;
  head_node = temp_node;
}

/*void display()
{
  int count = 0;
  temp_node = first_node;
  printf("\nDisplay Linked List : \n");
  while (temp_node != 0) {
    printf("# %d # ", temp_node->value);
    count++;
    temp_node = temp_node -> next;
  }
  printf("\nNo Of Items In Linked List : %d\n", count);
}*/

