# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider

def solution(x, t):
	if x >= 2 * t:
		return x * t - 0.5 * t * t + np.cos(np.pi * (2 * t - x))
	else:
		return x * t - 0.5 * t * t + 1 / 8 * (2 * t - x)**2 + np.exp(-t + 0.5 * x) 

N = 100
J = 250

x_1 = 0
x_2 = 1
T = 1

# calcY = np.zeros(shape = (J, N))

with open('output.txt', 'r') as f:
    calcY = np.array([[float(num) for num in line.split(',')] for line in f])
# print(calcY)
print(calcY.shape)

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
lineCalc, = plt.plot(x, calcY[0], 'r')
lineTrue, = plt.plot(x, trueY[0], 'g--')

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