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

    # plt.figure(figsize=(11.7,8.3))
    # plt.grid(which='both')
    # plt.grid(which='minor', alpha=0.2)
    # plt.grid(which='major', alpha=0.5)
    # plt.title('Stack benchmark, average operation time')
    # plt.minorticks_on()
    # plt.autoscale()
    # plt.xlabel("number of threads", fontsize=10)
    # plt.ylabel("time, ns", fontsize=10)

    numThreads = int(sys.argv[-1]) - 1
    for i in range(1, len(sys.argv) - 1):
        print('Input file is ' + sys.argv[i])
        with open(sys.argv[i]) as f:
            fileData = f.readlines()
            name = sys.argv[i]

            x = np.array([])
            averageYmutex = np.array([])
            maximumYmutex = np.array([])
            
            for j in range(1, 1 + numThreads):
                splitted = fileData[j].split(' ')
                print(splitted)
                x = np.append(x, float(splitted[0]))
                averageYmutex = np.append(averageYmutex, float(splitted[1]))
                maximumYmutex = np.append(maximumYmutex, float(splitted[2]))
            
            #plt.plot(x, averageY, label=name)
            #plt.plot(x, maximumY, label=name)

    #plt.legend()
    #plt.savefig("bench_spinlock_average.png")

    
    
    numThreads = int(sys.argv[-1])
    for i in range(1, len(sys.argv) - 1):
        with open(sys.argv[i]) as f:
            fileData = f.readlines()

            averageYfree= np.array([])
            maximumYfree = np.array([])
            
            for j in range(2 + numThreads, 1 + 2 * numThreads):
                splitted = fileData[j].split(' ')
                print(splitted)
                averageYfree = np.append(averageYfree, float(splitted[1]))
                maximumYfree = np.append(maximumYfree, float(splitted[2]))
            
            #plt.plot(x, averageY, label=name)
            #plt.plot(x, maximumY, label=name)

    
    
    plt.figure(figsize=(11.7,8.3))
    plt.grid(which='both')
    plt.grid(which='minor', alpha=0.2)
    plt.grid(which='major', alpha=0.5)
    plt.title('Lock Free/Mutex stack benchmark, average operation time')
    plt.minorticks_on()
    plt.autoscale()
    plt.xlabel("number of threads", fontsize=10)
    plt.ylabel("time, ns", fontsize=10)
    plt.plot(x, averageYmutex, label='mutex stack')
    plt.plot(x, averageYfree, label='lock free stack')
    plt.legend()
    plt.savefig("bench_stack_average.png")
    
    plt.figure(figsize=(11.7,8.3))
    plt.grid(which='both')
    plt.grid(which='minor', alpha=0.2)
    plt.grid(which='major', alpha=0.5)
    plt.title('Lock Free/Mutex stack benchmark, maximum operation time')
    plt.minorticks_on()
    plt.autoscale()
    plt.xlabel("number of threads", fontsize=10)
    plt.ylabel("time, ns", fontsize=10)
    plt.plot(x, maximumYmutex, label='mutex stack')
    plt.plot(x, maximumYfree, label='lock free stack')
    plt.legend()
    plt.savefig("bench_stack_maximum.png")
    
    # print(averageYmutex)
    # print(maximumYmutex)
    # print(averageYfree)
    # print(maximumYfree)

if __name__ == '__main__':
    main()