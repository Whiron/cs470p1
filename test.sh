#!/bin/bash
#
#SBATCH --job-name=par_gauss_omp
#SBATCH --nodes=1
#SBATCH --ntasks=1


make

echo "SERIAL:"
./gauss 500
./par_gauss 500

echo "PARALLEL:"
OMP_NUM_THREADS=1  ./par_gauss -t 500
OMP_NUM_THREADS=2  ./par_gauss -t 500
OMP_NUM_THREADS=4  ./par_gauss -t 500
OMP_NUM_THREADS=8  ./par_gauss -t 500
OMP_NUM_THREADS=16 ./par_gauss -t 500
