# -*- coding: utf-8 -*-
import sys
import os
import subprocess
import re

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

import random as rd
from datetime import datetime

def log(str):
    now = datetime.now()
    currentTime = now.strftime("%H:%M:%S")
    print("[" + currentTime + "]", str)

def generateArray(arraySize, minElement, maxElement, seed = 0):
    rd.seed(seed)

    inputArray = np.zeros(arraySize)
    inputString = str(arraySize) + " "
    for i in range(0, arraySize):
        number = rd.randint(minElement, maxElement)
        inputString += (str(number) + " ")
        inputArray[i] = number
    
    return inputArray, inputString

# Extract cpp programm from command line arguments
cProgram = sys.argv[1]
outputFile = "output.txt"

startCore = 1
endCore = 13
nCores = endCore - startCore + 1

# Generate input string (array size + array)
# arraySize = 15000000
maxElement = 1000000
minElement = -1000000

stepArraySize = 200000
startArraySize = 7000000
endArraySize = 11000000


nArraySize = int((endArraySize - startArraySize) / stepArraySize)

# (arraySize, nthreads, executionTime)

totalSize = nArraySize * nCores

coreArray = np.zeros(totalSize)
timeArray = np.zeros(totalSize)
sizeArray = np.zeros(totalSize)

count = 0
for arraySize in range(startArraySize, endArraySize, stepArraySize):
    seed = rd.randint(0, 1000)
    log("Generating input array, parameters: size = " + str(arraySize) + ", elements range = [" + str(minElement) + "; " + str(maxElement) + "]")
    inputArray, inputString = generateArray(arraySize, minElement, maxElement, seed)
    log("Succeded!")

    for n in range(startCore, endCore):
        log("Launching program with " + str(n) + " thread(s)")
        proc = subprocess.Popen([cProgram, str(n), outputFile], stdin = subprocess.PIPE, stdout = subprocess.PIPE)
        output, err = proc.communicate(input = inputString.encode())
        regResult = re.findall("\\d+\\.\\d+", str(output))
        
        # Fill final array
        coreArray[count] = n
        timeArray[count] = float(regResult[0])
        sizeArray[count] = arraySize

        log("Done! Execution time: " + str(timeArray[n - startCore]) + " seconds")

        # log("Comparing results with the actual answer...")
        # inputArray.sort()
        # resultArray = np.loadtxt(outputFile)

        # if (not np.array_equal(resultArray, inputArray)):
        #     log("Failed! Result is wrong")
        # else:
        #     log("Succeded!")

        count += 1

fig = plt.figure(figsize=(11.7, 8.3))

ax = fig.add_subplot(projection='3d')
ax.plot(timeArray, coreArray, sizeArray)
ax.set_title("Зависимость времени выполнения от кол-ва потоков и кол-ва элементов в массиве")
ax.set_xlabel("время, с")
ax.set_ylabel("кол-во потоков")
ax.set_zlabel("кол-во элементов")


plt.show()

# plt.figure(figsize=(11.7,8.3))
# plt.title("Зависимость времени выполнения от колличества ядер")
# plt.grid(which='both')
# plt.grid(which='minor', alpha=0.2)
# plt.grid(which='major', alpha=0.5)
# plt.minorticks_on()
# plt.autoscale()
# plt.xlabel("$n$, число ядер", fontsize=10)
# plt.ylabel("$T$, время выполнения", fontsize=10)
# plt.plot(nCoreArray, timeArray, 'o')
# plt.savefig("core_time.png")
# plt.show()