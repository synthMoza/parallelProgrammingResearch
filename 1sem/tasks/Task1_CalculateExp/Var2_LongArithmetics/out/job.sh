#!/bin/bash

#PBS -l walltime=00:01:00,nodes=5:ppn=4
#PBS -N Exp_Job
#PBS -q batch

N=1000000

cd $PBS_O_WORKDIR
mpirun --hostfile $PBS_NODEFILE -np 20 ./exp $N

