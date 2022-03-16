#!/bin/bash

#PBS -l walltime=00:01:00,nodes=1:ppn=2
#PBS -N Exp_Job
#PBS -q batch

N=894967295

cd $PBS_O_WORKDIR
mpirun --hostfile $PBS_NODEFILE -np 2 ./exp $N

