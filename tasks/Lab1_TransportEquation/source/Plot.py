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
cppProgram = sys.argv[1]

startCore = 2
endCore = 8
nCores = endCore - startCore + 1
timeArray = np.zeros(nCores)

for n in range(startCore, endCore + 1):
	proc = subprocess.Popen(["mpiexec", "-n", str(n), cppProgram, "output.txt"], stdout=subprocess.PIPE)
	output, error = proc.communicate()
	regResult = re.findall("\\d+\\.\\d+", str(output))
	timeArray[n - startCore] = float(regResult[0]);

# Plot (time, nCore)
nCoreArray = [i for i in range(2, 9)]

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

# Parse output file for further plotting
with open('output.txt', 'r') as f:
    calcY = np.array([[float(num) for num in line.split(',')] for line in f])

J = len(calcY)
N = len(calcY[0])	

x = [x_1 + i * (x_2 - x_1) / (N - 1) for i in range(0, N)]
t = [j * T / (J - 1) for j in range(0, J)]

trueY = np.zeros(shape = (J, N))
for i in range(0, N):
	for j in range(0, J):
		trueY[j][i] = solution(x[i], t[j])

plt.figure(figsize=(11.7,8.3))
plt.title("Сравнение аналитического и численного решений")
plt.grid(which='both')
plt.grid(which='minor', alpha=0.2)
plt.grid(which='major', alpha=0.5)
plt.minorticks_on()
plt.autoscale()
plt.xlabel("$x$", fontsize=10)
plt.ylabel("$y(x)$", fontsize=10)
lineCalc, = plt.plot(x, calcY[0], 'r', label='численное решение')
lineTrue, = plt.plot(x, trueY[0], 'g--', label='аналитическое решение')
plt.legend()
plt.subplots_adjust(bottom = 0.25)

# Ползунок для времени
axtime = plt.axes([0.25, 0.1, 0.65, 0.03])
timeSlider = Slider(
  ax = axtime,
  label = 'Время (в отсчетах)',
  valmin = 0,
  valmax = J - 1,
  valstep = 1,
  valinit = 0,
)

def update(val):
	lineCalc.set_ydata(calcY[val])
	lineTrue.set_ydata(trueY[val])

timeSlider.on_changed(update)
plt.show()