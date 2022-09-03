#!/bin/bash

#PBS -l walltime=00:01:00,nodes=2:ppn=3
#PBS -N Proxy_Job
#PBS -q batch

cd $PBS_O_WORKDIR
mpirun --hostfile $PBS_NODEFILE -np 6 ./proxy