# Compiling and running
## MPI
To launch any task and see its result, type in the command line:
```
cd tasks/Task#_<name>/out
make
make cat
```

```make``` compiles the program and puts it into the queue, and ```make cat``` displays the result from the files ```<job_name>.o<number>``` and ```<job_name>.e<number>```. The typical result of ```make cat```:
```
Errors:
Output:
Thread[1]: Modified value of message is 2
Thread[2]: Modified value of message is 3
Thread[0]: Initial value of the message is 1
Thread[0]: Final value of message is 6
Thread[3]: Modified value of message is 4
Thread[4]: Modified value of message is 5
...
```
## Posix Threads
To launch any task and see its result, type in the command line:
```
mkdir build
cd build
cmake ..
make
```


# List of tasks
## Task 0
### Hello World (MPI)
Each thread must print its rank and communicator size.
```
Communicator size = 28
My rank = 0
Communicator size = 28
My rank = 1
Communicator size = 28
My rank = 3
Communicator size = 28
My rank = 8
Communicator size = 28
My rank = 10
Communicator size = 28
My rank = 2
Communicator size = 28
My rank = 11
Communicator size = 28
My rank = 19
Communicator size = 28
My rank = 24
Communicator size = 28
My rank = 16
Communicator size = 28
My rank = 4
Communicator size = 28
My rank = 25
...
```
### Harmonic Series (MPI)
Given number N as an command argument, calculate the harmonic series up to Nth term.
```
===========================
        WORKERS: 28
        INPUT NUM: 100000
        RESULT: 12.0901
===========================
```
### Proxy (MPI)
Some thread (let's call it the first one) has to initialize some message (string or number) and send to the next thread, which has to modify it somehow and send it next. The last one has to return it to the first one. Each thread has to log all receives and sends.
```
Thread[1]: Modified value of message is 2
Thread[2]: Modified value of message is 3
Thread[0]: Initial value of the message is 1
Thread[0]: Final value of message is 6
Thread[3]: Modified value of message is 4
Thread[4]: Modified value of message is 5
Thread[5]: Modified value of message is 6
Thread[0]: Initial value of the message is 1
Thread[1]: Modified value of message is 2
Thread[2]: Modified value of message is 3
...
```
## Task 1
### Calculate exponent, variant 1 (MPI)
Given N as an argument - number of members to calculate using exponent Taylor series, calculate the value of e constant.
```
Given argument: N = 894967295
Result: e = 2.7182818284590452354
```

Time measurement:

- 4 threads - 16.6 seconds
- 3 threads - 17.9 seconds
- 2 threads - 20.7 seconds
- 1 thread  - 38.2 seconds
### Calculate exponent, variant 2 (MPI)
Given N as an argument - number of digits after the comma to be calculated, calculate the value of e constant.
```
Result: e = 2.718281828459045235360287471352662497757247093699959574966967627724...
Runtime = 32.2859 s
```
Time measurement (million digits):

- 4 threads - 58.6 seconds
- 8 threads - 42.1 seconds
- 12 threads - 36.8 seconds
- 16 threads - 34.3 seconds
- 20 threads - 32.4 seconds

## Lab 1 - Transport Equation (MPI)
Solve the transport equation using one of availible calculation schemes (the implemented one is "square"). Includes plots - comparing calculation result and the real solution, compare number of cores to execution time.


<img src="tasks/Lab1_TransportEquation/core_time.png" width="585" height="415">

![image](tasks/Lab1_TransportEquation/calc_to_true_comparison.gif)
## Lab 2 - Integral Calculation (Posix Threads)
Calculate integral of f(x) = sin(1/x) from some small posiive number to some big positive number (for example, [0.015, 1e6]). Thread must waste equal time and use dynamic step while calculating. Example output:
```
Thread 0x7efee8e93700: took 7.271794 sec
Thread 0x7efee8692700: took 7.386910 sec
Thread 0x7efee7e91700: took 7.589306 sec
Thread 0x7efee9694700: took 7.744253 sec
Number of threads: 4
Result: 14.2385
```
<img src="tasks/Lab2_IntegralCalculation/core_time.png" width="585" height="415">

## Task 2

### Parallel Sorting (Posix Threads)

Use some parallel algorithm to sort the given array. This implementation uses quick sort on ```n``` parts of the given array, where ```n``` is the number of threads, and then merges them (like in merge sort). Script generates random arrays, uses several seeds and array size, plots 3D graphic to show acceleration. Program takes number of threads as an argument, and filename as an optional argument (if no file provided - print the array to stdout). Then program takes array size and array itself. Example:

```
./parallelSort 5 output.txt
6 3 -4 2 0 5 1
cat output.txt
-4 0 1 2 3 5
```