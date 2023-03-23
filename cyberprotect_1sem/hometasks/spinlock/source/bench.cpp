#include <spin_lock.h>
#include <vector>
#include <thread>
#include <iostream>
#include <cstring>

using namespace async;

/*
    Average access time - average time per thread to take the spin lock
    Maximum access time - maximum time per thread to take the spin lock
*/

constexpr size_t threadBegin = 2;
constexpr size_t threadEnd = 12;

using namespace std::chrono_literals;

void work(ISpinLock& spin, double& time)
{
    auto start = std::chrono::high_resolution_clock::now();
    
    spin.Lock();
    
    // measure how much time it takes to take spin lock
    auto end = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(800ns);

    spin.Unlock();
    
    time = (end - start).count();
}

void BenchmarkSpinlockAverage(ISpinLock& spin)
{
    for (size_t i = threadBegin; i < threadEnd; ++i)
    {
        // check all thread numbers
        std::cout << i << " ";

        std::vector<std::thread> threads;
        std::vector<double> times(i);

        for (size_t j = 0; j < i; ++j)
            threads.push_back(std::thread(work, std::ref(spin), std::ref(times[j])));

        for (auto& thread : threads)
            thread.join();

        double average = 0;
        for (auto& time : times)
            average += time;
        
        average /= i;
        std::cout << average << std::endl;
    }
}

void BenchmarkSpinlockMaximum(ISpinLock& spin)
{
    for (size_t i = threadBegin; i < threadEnd; ++i)
    {
        // check all thread numbers
        std::cout << i << " ";

        std::vector<std::thread> threads;
        std::vector<double> times(i);

        for (size_t j = 0; j < i; ++j)
            threads.push_back(std::thread(work, std::ref(spin), std::ref(times[j])));

        for (auto& thread : threads)
            thread.join();

        double maximum = 0;
        for (auto& time : times)
            if (time > maximum)
                maximum = time;
        
        std::cout << maximum << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./spin_lock_bench <spin_lock_name>" << std::endl;
        return -1;
    }

    ISpinLock* spinLock = nullptr;
    if (std::strcmp(argv[1], "tas") == 0)
    {
        spinLock = new SpinLockTAS;
    }
    else if (std::strcmp(argv[1], "ttas") == 0)
    {
        spinLock = new SpinLockTTAS;
    }
    else if (std::strcmp(argv[1], "ticket") == 0)
    {
        spinLock = new SpinLockTicket;
    }
    else
    {
        std::cerr << "Unknown spin lock " << argv[1] << ", aborting" << std::endl;
        return -1;
    }

    std::cout << "average" << std::endl;
    BenchmarkSpinlockAverage(*spinLock);
    std::cout << "\nmaximum" << std::endl;
    BenchmarkSpinlockMaximum(*spinLock);

    return 0;
}
