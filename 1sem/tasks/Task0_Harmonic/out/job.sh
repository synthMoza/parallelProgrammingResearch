#!/bin/bash

#PBS -l walltime=00:01:00,nodes=7:ppn=4
#PBS -N Harmonic_Series_Job
#PBS -q batch

N=100000

cd $PBS_O_WORKDIR
mpirun --hostfile $PBS_NODEFILE -np 28 ./harmonic $N

