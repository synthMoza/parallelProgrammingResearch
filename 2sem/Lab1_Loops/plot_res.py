import sys

import numpy as np
import matplotlib.pyplot as plt

def plot(x, y):
    plt.figure(figsize=(11.7,8.3))
    plt.grid(which='both')
    plt.grid(which='minor', alpha=0.2)
    plt.grid(which='major', alpha=0.5)
    plt.title('Time dependency on threads count')
    plt.minorticks_on()
    plt.autoscale()
    plt.xlabel("number of threads", fontsize=10)
    plt.ylabel("time, s", fontsize=10)
    plt.plot(x, y, 'bo')
    plt.plot(x, y, 'r--')
    plt.savefig(sys.argv[1] + '.png')

def main():
    if len(sys.argv) != 3:
        print('Error! Wrong input, usage: python plot_res.py <name> <file>')
        exit(-1)
    
    with open(sys.argv[2]) as f:
        lines = f.readlines()

    threads = np.array([])
    times = np.array([])

    for i in range(0, len(lines), 2):
        threads = np.append(threads, int(lines[i]))
        times = np.append(times, float(lines[i+1].split()[-2]))

    plot(threads, times)

if __name__ == '__main__':
    main()