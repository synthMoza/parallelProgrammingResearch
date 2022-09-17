import json
import sys

import numpy as np
import matplotlib.pyplot as plt

def plotBenchGraph(name, x, y):
    plt.figure(figsize=(11.7,8.3))
    plt.grid(which='both')
    plt.grid(which='minor', alpha=0.2)
    plt.grid(which='major', alpha=0.5)
    plt.title(name + ', CPU time dependency on iteration value')
    plt.minorticks_on()
    plt.autoscale()
    plt.xlabel("iteration value", fontsize=10)
    plt.ylabel("CPU time, ns", fontsize=10)
    plt.plot(x, y, 'bo')
    plt.plot(x, y, 'r--')
    plt.savefig(name + '.png')

def main():
    benchmarkNames = ['BM_Matrix_DiffSizeSameThreadNum', 'BM_Matrix_SameSizeDiffThreadNum']

    if len(sys.argv) != 2:
        print('Error! No input file provided, usage: python analyze_bench.py <input_bench_json>')
        exit(-1)
    
    print('Input JSON file is ' + sys.argv[1])
    with open(sys.argv[1]) as f:
        fileData = f.read()

    dictData = json.loads(fileData)
    print('Successfuly read JSON data!')

    for benchName in benchmarkNames:        
        print('Current benchmark: ' + benchName)

        benchCPUtime = np.array([])
        benchSizes = np.array([])

        for bench in dictData['benchmarks']:
            # Нужен только текущий бенчмарк, время бенчмарка есть только в итерациях
            if (bench['run_type'] != 'iteration' or benchName not in bench['name']):
                continue
            
            benchCPUtime = np.append(benchCPUtime, bench['cpu_time'])
            # Извлекаем кол-во элементов в матрице
            benchSizes = np.append(benchSizes, int(bench['name'].replace(benchName + '/', '')))

        # Строим график
        plotBenchGraph(benchName, benchSizes, benchCPUtime)

        print('Benchmark CPU time array:')
        print(benchCPUtime)

        print('Benchmark iteration values array:')
        print(benchSizes)

        if benchName == 'BM_Matrix_SameSizeDiffThreadNum':
            print('Acceleration for each iteration:')
            acc = np.ones(len(benchCPUtime)) * benchCPUtime[0] / benchCPUtime
            print(acc)

            print('Efficiency for each iteration:')
            print(acc / benchSizes)


if __name__ == '__main__':
    main()