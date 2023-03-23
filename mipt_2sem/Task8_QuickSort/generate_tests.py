import numpy as np
from pathlib import Path

arraySize = 100000
testDir = 'tests'
testCounter = 0

# Создаем выходную директорию тестов
Path(testDir).mkdir(parents=True, exist_ok=True)

def GetTestPath():
    global testCounter

    string = testDir + '/' + 'test_' + str(testCounter) + '.txt'
    testCounter += 1

    return string

# Первый тест - исходно отсортированный массив
arr = [i for i in range(0, arraySize)]
np.savetxt(GetTestPath(), arr, fmt  = '%d')

# Второй тест - обратно сортированный массив
arr = [arraySize - i for i in range(0, arraySize)]
np.savetxt(GetTestPath(), arr, fmt  = '%d')

# Третий тест - рандомный массив
arr = np.random.randint(-arraySize, arraySize, arraySize)
np.savetxt(GetTestPath(), arr, fmt  = '%d')
