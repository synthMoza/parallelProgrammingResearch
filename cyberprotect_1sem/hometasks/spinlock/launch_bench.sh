#!/bin/bash

./$1/spin_lock_bench tas > bench_tas.txt
./$1/spin_lock_bench ttas > bench_ttas.txt
./$1/spin_lock_bench ticket > bench_ticket.txt

python3 analyze_bench.py bench_tas.txt bench_ttas.txt bench_ticket.txt 10

