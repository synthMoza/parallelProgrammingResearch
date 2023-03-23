import sys

import numpy as np
import matplotlib.pyplot as plt

def plot(x, y, name):
    plt.figure(figsize=(11.7,8.3))
    plt.grid(which='both')
    plt.grid(which='minor', alpha=0.2)
    plt.grid(which='major', alpha=0.5)
    plt.title(name + ' dependency on threads count')
    plt.minorticks_on()
    plt.autoscale()
    plt.xlabel("number of threads", fontsize=10)
    plt.ylabel("time, s", fontsize=10)
    plt.plot(x, y, 'bo')
    plt.plot(x, y, 'r--')
    plt.savefig(sys.argv[1] + name + '.png')

def main():
    if len(sys.argv) != 3:
        print('Error! Wrong input, usage: python plot_res.py <name> <file>')
        exit(-1)
    
    with open(sys.argv[2]) as f:
        lines = f.readlines()

    threads = np.array([])
    times = np.array([])
    acceleration = np.array([])
    effeciency = np.array([])

    oneThreadTime = 0 # time on one thread for acceleration and effeciency calculation
    for i in range(0, len(lines), 2):
        if i == 0:
            oneThreadTime = float(lines[i+1].split()[-2])
        threads = np.append(threads, int(lines[i]))
        times = np.append(times, float(lines[i+1].split()[-2]))
        acceleration = np.append(acceleration,  oneThreadTime / float(lines[i+1].split()[-2]))
        effeciency = np.append(effeciency, oneThreadTime / (float(lines[i+1].split()[-2]) * int(lines[i])))

    plot(threads, times, 'Time')
    plot(threads, acceleration, 'Acceleration')
    plot(threads, effeciency, 'Effeciency')

if __name__ == '__main__':
    main()