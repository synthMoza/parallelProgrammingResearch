#include <mutex_stack.h>
#include <lock_free_stack.h>

#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <utility>

using namespace util;

using time_pair_t = std::pair<size_t, size_t>;

// perform push/pop with 1/2 probability
template <typename Stack>
time_pair_t BenchmarkRandom(Stack& stack, size_t size)
{
    size_t totalTime = 0;
    size_t maxTime = 0;

    for (size_t i = 0; i < size; ++i)
    {
        int rd = random() % 2;

        auto start = std::chrono::high_resolution_clock::now();

        if (rd == 0)
            stack.push(i);
        else
            (void) stack.pop();
        
        auto end = std::chrono::high_resolution_clock::now();
        
        size_t current = (end - start).count(); 
        if (current > maxTime)
            maxTime = current;
        totalTime += current;
    }

    totalTime /= size; // average time for THIS thread
    return time_pair_t{totalTime, maxTime};
}

// size/2 pushes, size/2 pops
template <typename Stack>
time_pair_t BenchmarkPushPop(Stack& stack, size_t size)
{
    size_t totalTime = 0;
    size_t maxTime = 0;

    for (size_t i = 0; i < size; ++i)
    {
        auto start = std::chrono::high_resolution_clock::now();

        if (i <= size / 2)
            stack.push(i);
        else
            (void) stack.pop();
        
        auto end = std::chrono::high_resolution_clock::now();
        
        size_t current = (end - start).count(); 
        if (current > maxTime)
            maxTime = current;
        totalTime += current;
    }

    totalTime /= size; // average time for THIS thread
    return time_pair_t{totalTime, maxTime};
}

constexpr size_t countOperations = 1000000;

template <typename Stack>
void DoBenchRandom(const char* name)
{
    std::cout << name << std::endl;
    for (size_t numThreads = 1; numThreads < 10; ++numThreads)
    {
        std::vector<time_pair_t> threadTime(numThreads);
        std::vector<std::thread> threads;
        
        Stack stack;
        for (size_t i = 0; i < numThreads; ++i)
        {
            threads.push_back(std::thread([&](size_t idx){
                auto pair = BenchmarkRandom(stack, countOperations);
                threadTime.at(idx) = pair;
            }, i));
        }

        size_t averageTime = 0;
        size_t maxTime = 0;
        for (size_t i = 0; i < numThreads; ++i)
        {
            threads.at(i).join();
            averageTime += threadTime.at(i).first;
            if (maxTime < threadTime.at(i).second)
                maxTime = threadTime.at(i).second;
        }

        averageTime /= numThreads;
        // std::cout << "50/50 push/pop; number of threads: " << numThreads << ", average time to perform op: " << averageTime 
        // << " ns, maximum time to perform op: " << maxTime << " ns" << std::endl;
        std::cout << numThreads << " " << averageTime << " " << maxTime << std::endl;
    }
}

template <typename Stack>
void DoBenchPushPop(const char* name)
{
    std::cout << name << std::endl;
    for (size_t numThreads = 1; numThreads < 10; ++numThreads)
    {
        std::vector<time_pair_t> threadTime(numThreads);
        std::vector<std::thread> threads;
        
        Stack stack;
        for (size_t i = 0; i < numThreads; ++i)
        {
            threads.push_back(std::thread([&](size_t idx){
                auto pair = BenchmarkPushPop(stack, countOperations);
                threadTime.at(idx) = pair;
            }, i));
        }

        size_t averageTime = 0;
        size_t maxTime = 0;
        for (size_t i = 0; i < numThreads; ++i)
        {
            threads.at(i).join();
            averageTime += threadTime.at(i).first;
            if (maxTime < threadTime.at(i).second)
                maxTime = threadTime.at(i).second;
        }

        averageTime /= numThreads;
        // std::cout << "50/50 push/pop; number of threads: " << numThreads << ", average time to perform op: " << averageTime 
        // << " ns, maximum time to perform op: " << maxTime << " ns" << std::endl;
        std::cout << numThreads << " " << averageTime << " " << maxTime << std::endl;
    }
}

int main(int argc, char* argv[])
{
    (void) argc;

    if (argv[1][0] == 'r')
    {
        DoBenchRandom<MutexStack<int>>("Mutex Stack");
        DoBenchRandom<LockFreeStack<int>>("Lock Free Stack");
    }
    else if (argv[1][0] == 'p')
    {
        DoBenchPushPop<MutexStack<int>>("Mutex Stack");
        DoBenchPushPop<LockFreeStack<int>>("Lock Free Stack");
    }

    return 0;   
}