#!/bin/bash

#PBS -l walltime=00:05:00,nodes=7:ppn=4
#PBS -N synthMozaJob
#PBS -q batch

cd $PBS_O_WORKDIR
	
echo $NUM_THREADS
mpirun --hostfile $PBS_NODEFILE -np $NUM_THREADS ./../build/generic_parallel

