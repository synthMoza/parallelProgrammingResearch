#include <spin_lock.h>
#include <thread>
#include <vector>
#include <iostream>

using namespace async;

constexpr size_t g_threadsNum = 50;
using SpinLockType = SpinLockTAS;

void work(SpinLockType& spinLock, size_t idx)
{
    spinLock.Lock();
    std::cout << "My name is " << idx << "and you?" << std::endl;
    spinLock.Unlock();
}

int main()
{
    SpinLockType spinLock;
    std::vector<std::thread> threads;

    threads.reserve(g_threadsNum);
    
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < g_threadsNum; ++i)
        threads.emplace_back(work, std::ref(spinLock), i);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "It took " << (end - start).count() << " nanoseconds, wow" << std::endl;

    for (auto& thread : threads)
        thread.join();

    return 0;
}