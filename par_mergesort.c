/*
 * par_gauss.c
 *
 * CS 470 Project 3 (OpenMP)
 * OpenMP parallelized version
 * Authors: Georgia Corey & Justin Mikesell
 * Compile with --std=c99
 *
 * rand_system() analysis: We set up a parallel construct to execute the two for loops within 
 * the function to define the shared variables (there were no private variables). We then created
 * two parallel loops which specified that the loop iterations would be distributed among and 
 * executed by all threads passed to it. Additionally, we added in an OpenMP check to set seed to 
 * the number of threads if it was being executed in OpenMP, otherwise, seed was initialized to 0. 
 * As a result of this, seed became the function’s only private variable. In terms of scaling, if 
 * the thread count increases and the matrix is at 500, there is speedup up until 8 threads, where
 * it levels out at about 0.0006.
 *
 * gausssian_elimination() analysis: For this function, we had to declare the parallel for loop inside
 * of the first for loop. We at first attempted a collapse, however, this gave us severely inaccurate 
 * results and no scaling of any kind. Another attempt we made to achieve optimal parallelization was 
 * to make one or more variables private, but like the attempt at collapsing the nested for loops, this
 * almost provided inaccurate results with no scaling. We also tried to use a reduction clause, which 
 * seemed to make the program take longer and lost all scaling. We found that using a parallel for 
 * declaration was the best method to achieve optimal parallelization in this function. In terms of 
 * scaling at a matrix size of 500, the function demonstrated strong scaling throughout, almost linear.
 *
 * back_substitution_ row() and back_substitution_col() analysis: We struggled a decent amount to try and
 * parallelize these functions. In our many attempts we tried to use a pragma omp parallel for which showed
 * strong scaling for the function, but created incorrect results. We then tried to implement it using a 
 * seperate pragma omp parallel and then pragma omp for but we got the incorrect results. In an attempt 
 * to get something working we placed pragma omp for’s at each for loop, but that did not improve scaling, 
 * however it still created correct results.
 *
 */

#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif
// custom timing macros
#include "omp_timer.h"

// uncomment this line to enable the alternative back substitution method
/*#define USE_COLUMN_BACKSUB*/

// use 64-bit IEEE arithmetic (change to "float" to use 32-bit arithmetic)
#define REAL double

// linear system: Ax = b    (A is n x n matrix; b and x are n x 1 vectors)
int n;
REAL *A;
REAL *x;
REAL *b;

// enable/disable debugging output (don't enable for large matrix sizes!)
bool debug_mode = false;

// enable/disable triangular mode (to skip the Gaussian elimination phase)
bool triangular_mode = false;

/*
 * Generate a random linear system of size n.
 */
void rand_system()
{
 // allocate space for matrices
    A = (REAL*)calloc(n*n, sizeof(REAL));
    b = (REAL*)calloc(n,   sizeof(REAL));
    x = (REAL*)calloc(n,   sizeof(REAL));

    // verify that memory allocation succeeded
    if (A == NULL || b == NULL || x == NULL) {
        printf("Unable to allocate memory for linear system\n");
        exit(EXIT_FAILURE);
    }

    // initialize pseudorandom number generator
    // (see https://en.wikipedia.org/wiki/Linear_congruential_generator)

    // Parallelize the function
 #  pragma omp parallel default(none) \
    shared(A,b, x, n, triangular_mode)
    {
// Assign seed to its own thread if OpenMP is running
// Otherwise, initialize seed to 0.
    unsigned long seed;
#   ifdef _OPENMP
    seed = omp_get_thread_num();
#   else
    seed = 0;
#   endif
    // generate random matrix entries
    // Parallelize the for loops
#   pragma omp for
    for (int row = 0; row < n; row++) {
        int col = triangular_mode ? row : 0;
       for (; col < n; col++) {
            if (row != col) {
                seed = (1103515245*seed + 12345) % (1<<31);
                A[row*n + col] = (REAL)seed / (REAL)ULONG_MAX;
            }
            else {
                A[row*n + col] = n/10.0;
            }
        }
    }

    // generate right-hand side such that the solution matrix is all 1s
    // Parallelize the for loops
#   pragma omp for
    for (int row = 0; row < n; row++) {
        b[row] = 0.0;
        for (int col = 0; col < n; col++) {
            b[row] += A[row*n + col] * 1.0;
        }
    }
 }
}

/*
 * Reads a linear system of equations from a file in the form of an augmented
 * matrix [A][b].
 */
void read_system(const char *fn)
{
    // open file and read matrix dimensions
    FILE* fin = fopen(fn, "r");
    if (fin == NULL) {
        printf("Unable to open file \"%s\"\n", fn);
        exit(EXIT_FAILURE);
    }
    if (fscanf(fin, "%d\n", &n) != 1) {
        printf("Invalid matrix file format\n");
        exit(EXIT_FAILURE);
    }

    // allocate space for matrices
    A = (REAL*)malloc(sizeof(REAL) * n*n);
    b = (REAL*)malloc(sizeof(REAL) * n);
    x = (REAL*)malloc(sizeof(REAL) * n);

    // verify that memory allocation succeeded
    if (A == NULL || b == NULL || x == NULL) {
        printf("Unable to allocate memory for linear system\n");
        exit(EXIT_FAILURE);
    }

    // read all values
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            if (fscanf(fin, "%lf", &A[row*n + col]) != 1) {
                printf("Invalid matrix file format\n");
                exit(EXIT_FAILURE);
            }
        }
        if (fscanf(fin, "%lf", &b[row]) != 1) {
            printf("Invalid matrix file format\n");
            exit(EXIT_FAILURE);
        }
        x[row] = 0.0;     // initialize x while we're reading A and b
    }
    fclose(fin);
}

/*
 * Performs Gaussian elimination on the linear system.
 * Assumes the matrix is singular and doesn't require any pivoting.
 */
void gaussian_elimination()
{
   // Gaussian elimination for loopss. Parallelized on the first nested for loop.
   for (int pivot = 0; pivot < n; pivot++) {
#      pragma omp parallel for default(none) shared(A, b, n, pivot)
       for (int row = pivot+1; row < n; row++) {
            REAL coeff = A[row*n + pivot] / A[pivot*n + pivot];
            A[row*n + pivot] = 0.0;
            for (int col = pivot+1; col < n; col++) {
                A[row*n + col] -= A[pivot*n + col] * coeff;
            }
            b[row] -= b[pivot] * coeff;
        }
    }
}

/*
 * Performs backwards substitution on the linear system.
 * (row-oriented version)
 */
void back_substitution_row()
{
    REAL tmp;
    // Parallelize the for loop
#   pragma omp for
    for (int row = n-1; row >= 0; row--) {
        tmp = b[row];
        for (int col = row+1; col < n; col++) {
            tmp += -A[row*n + col] * x[col];
        }
        x[row] = tmp / A[row*n + row];
    }
}

/*
 * Performs backwards substitution on the linear system.
 * (column-oriented version)
 */
void back_substitution_column()
{
    // Both for loops are parallelized.
#   pragma omp for
    for (int row = 0; row < n; row++) {
        x[row] = b[row];
    }
#   pragma omp for
    for (int col = n-1; col >= 0; col--) {
        x[col] /= A[col*n + col];
        for (int row = 0; row < col; row++) {
            x[row] += -A[row*n + col] * x[col];
        }
    }
}

/*
 * Find the maximum error in the solution (only works for randomly-generated
 * matrices).
 */
REAL find_max_error()
{
    REAL error = 0.0, tmp;
    for (int row = 0; row < n; row++) {
        tmp = fabs(x[row] - 1.0);
        if (tmp > error) {
            error = tmp;
        }
    }
    return error;
}

/*
 * Prints a matrix to standard output in a fixed-width format.
 */
void print_matrix(REAL *mat, int rows, int cols)
{
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            printf("%8.1e ", mat[row*cols + col]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    // check and parse command line options
    int threads;
#   ifdef _OPENMP
    threads = omp_get_max_threads();
#   else
    threads = 0;
#   endif
    int c;
    while ((c = getopt(argc, argv, "dt")) != -1) {
        switch (c) {
        case 'd':
            debug_mode = true;
            break;
        case 't':
            triangular_mode = true;
            break;
        default:
            printf("Usage: %s [-dt] <file|size>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (optind != argc-1) {
        printf("Usage: %s [-dt] <file|size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // read or generate linear system
    long int size = strtol(argv[optind], NULL, 10);
    START_TIMER(init)
    if (size == 0) {
        read_system(argv[optind]);
    } else {
        n = (int)size;
        rand_system();
    }
    STOP_TIMER(init)

    if (debug_mode) {
        printf("Original A = \n");
        print_matrix(A, n, n);
        printf("Original b = \n");
        print_matrix(b, n, 1);
    }

    // perform gaussian elimination
    START_TIMER(gaus)
    if (!triangular_mode) {
        gaussian_elimination();
    }
    STOP_TIMER(gaus)

    // perform backwards substitution
    START_TIMER(bsub)
#   ifndef USE_COLUMN_BACKSUB
    back_substitution_row();
#   else
    back_substitution_column();
#   endif
    STOP_TIMER(bsub)

    if (debug_mode) {
        printf("Triangular A = \n");
        print_matrix(A, n, n);
        printf("Updated b = \n");
        print_matrix(b, n, 1);
        printf("Solution x = \n");
        print_matrix(x, n, 1);
    }

    // print results
    printf("Nthreads=%2d  ERR=%8.1e  INIT: %8.4fs  GAUS: %8.4fs  BSUB: %8.4fs\n",
            threads, find_max_error(),
            GET_TIMER(init), GET_TIMER(gaus), GET_TIMER(bsub));

    // clean up and exit
    free(A);
    free(b);
    free(x);
    return EXIT_SUCCESS;
}

