# -*- coding: utf-8 -*-
import sys
import os
import subprocess
import re

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider

# Initial conditions
x_1 = 0
x_2 = 1
T = 1

def solution(x, t):
	if x >= 2 * t:
		return x * t - 0.5 * t * t + np.cos(np.pi * (2 * t - x))
	else:
		return x * t - 0.5 * t * t + 1 / 8 * (2 * t - x)**2 + np.exp(-t + 0.5 * x) 


# Extract cpp programm from command line arguments
cProgram = sys.argv[1]

startCore = 1
endCore = 12
nCores = endCore - startCore + 1
timeArray = np.zeros(nCores)

for n in range(startCore, endCore + 1):
	proc = subprocess.Popen(["time", cProgram, str(n)], stdout=subprocess.PIPE)
	output, error = proc.communicate()
	regResult = re.findall("\\d+\\.\\d+", str(output))
	timeArray[n - startCore] = float(regResult[0])

# Plot (time, nCore)
nCoreArray = [i for i in range(startCore, endCore + 1)]

plt.figure(figsize=(11.7,8.3))
plt.title("Зависимость времени выполнения от колличества ядер")
plt.grid(which='both')
plt.grid(which='minor', alpha=0.2)
plt.grid(which='major', alpha=0.5)
plt.minorticks_on()
plt.autoscale()
plt.xlabel("$n$, число ядер", fontsize=10)
plt.ylabel("$T$, время выполнения", fontsize=10)
plt.plot(nCoreArray, timeArray, 'o')
plt.savefig("core_time.png")
plt.show()