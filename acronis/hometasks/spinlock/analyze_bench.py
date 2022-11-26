import sys

import numpy as np
import matplotlib.pyplot as plt

def plotBenchGraph(name, x, y):
   
    plt.plot(x, y, 'bo')
    plt.plot(x, y, 'r--')
    plt.savefig(name + '.png')

def main():
    if len(sys.argv) < 3:
        print('Input error! Usage: python analyze_bench.py <bench files> <num_threads>')
        exit(-1)

    plt.figure(figsize=(11.7,8.3))
    plt.grid(which='both')
    plt.grid(which='minor', alpha=0.2)
    plt.grid(which='major', alpha=0.5)
    plt.title('Spin Lock benchmark, average acquire time')
    plt.minorticks_on()
    plt.autoscale()
    plt.xlabel("number of threads", fontsize=10)
    plt.ylabel("time, ns", fontsize=10)

    numThreads = int(sys.argv[-1])
    for i in range(1, len(sys.argv) - 1):
        print('Input file is ' + sys.argv[i])
        with open(sys.argv[i]) as f:
            fileData = f.readlines()
            spinLockName = sys.argv[i].split('_')[1]

            averageX = np.array([])
            averageY = np.array([])
            
            for j in range(1, 1 + numThreads):
                splitted = fileData[j].split(' ')
                averageX = np.append(averageX, float(splitted[0]))
                averageY = np.append(averageY, float(splitted[1]))
            
            plt.plot(averageX, averageY, label=spinLockName)            

    plt.legend()
    plt.savefig("bench_spinlock_average.png")

    plt.figure(figsize=(11.7,8.3))
    plt.grid(which='both')
    plt.grid(which='minor', alpha=0.2)
    plt.grid(which='major', alpha=0.5)
    plt.title('Spin Lock benchmark, maximum acquire time')
    plt.minorticks_on()
    plt.autoscale()
    plt.xlabel("number of threads", fontsize=10)
    plt.ylabel("time, ns", fontsize=10)

    numThreads = int(sys.argv[-1])
    for i in range(1, len(sys.argv) - 1):
        print('Input file is ' + sys.argv[i])
        with open(sys.argv[i]) as f:
            fileData = f.readlines()
            spinLockName = sys.argv[i].split('_')[1]

            maximumX = np.array([])
            maximumY = np.array([])
            
            for j in range(3 + numThreads, 3 + 2 * numThreads):
                splitted = fileData[j].split(' ')
                maximumX = np.append(maximumX, float(splitted[0]))
                maximumY = np.append(maximumY, float(splitted[1]))
            
            plt.plot(maximumX, maximumY, label=spinLockName)            

    plt.legend()
    plt.savefig("bench_spinlock_maximum.png")

if __name__ == '__main__':
    main()