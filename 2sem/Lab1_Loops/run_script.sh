#!/bin/bash

for i in {1..32}
do
	qsub -v NUM_THREADS=$i job.sh
	if (( $i % 5 == 0)); then
		sleep 90
	fi
done
