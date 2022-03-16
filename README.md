# About
This repository contains tasks and their solutions from the Parallel Programming MIPT course.
# Structure
All tasks and their solution are located in the folder ```tasks```. Eack folder name is ```Task#_<name>```, where ```#``` is the number of the task, ```name``` is the brief name of the task. Each task folder has two internal folders - ```out``` and ```source```. ```source``` contains all source files written on C using MPI library, ```out``` folder contains ```Makefile``` for compilation and ```job.sh``` - bash script for putting program in a queue (PBS TORQUE).
# Compiling and running
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
# List of tasks
## Task 0
### Hello World
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
### Harmonic Series
Given number N as an command argument, calculate the harmonic series up to Nth term.
```
===========================
        WORKERS: 28
        INPUT NUM: 100000
        RESULT: 12.0901
===========================
```
### Proxy
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
### Calculate exponent, variant 1
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
