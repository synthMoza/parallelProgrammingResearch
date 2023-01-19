#ifndef THREAD_POOL_HEADER
#define THREAD_POOL_HEADER

#include <vector>
#include <deque>
#include <mutex>
#include <future>
#include <functional>
#include <utility>
#include <thread>

#include <iostream>

namespace util
{


using task_t = std::function<void()>;
using task_deque_t = std::deque<task_t>;

constexpr size_t MAX_STEAL_ATTEMPTS = 3;

class ThreadPool
{
    bool m_stop;
    std::vector<std::thread> m_workers;

    std::vector<task_deque_t> m_taskDeques;
    std::vector<std::mutex> m_mutexes;

    // get thread with the max amount of tasks
    // as we don't block all queus to calculate that, it will be only 
    // approximately the best thread to steal from
    size_t GetVictimThread()
    {
        size_t victimThread = rand() % m_workers.size();
        for (size_t i = 0; i < m_workers.size(); ++i)
            if (m_taskDeques[i].size() > m_taskDeques[victimThread].size())
                victimThread = i;
        
        return victimThread;
    }

    // get thread with the least amount of tasks to given next task
    size_t GetNextThread()
    {
        size_t netxThread = rand() % m_workers.size();
        for (size_t i = 0; i < m_workers.size(); ++i)
            if (m_taskDeques[i].size() < m_taskDeques[netxThread].size())
                netxThread = i;
        
        return netxThread;
    }

public:
    ThreadPool(size_t numThreads = std::thread::hardware_concurrency()) :
        m_stop(false),
        m_taskDeques(numThreads),
        m_mutexes(numThreads)
    {
        for (size_t i = 0; i < numThreads; ++i)
            m_workers.emplace_back([this](size_t idx)
            {
                while (!m_stop)
                {
                    // perform this thread tasks
                    {
                        while (!m_taskDeques[idx].empty())
                        {
                            task_t currentTask;
                            {
                                m_mutexes[idx].lock();
                                // there may be a race between check of emptiness and acquiring mutex
                                if (m_taskDeques[idx].empty())
                                {
                                    m_mutexes[idx].unlock();
                                    break;
                                }
                                
                                std::cout << "queue size: " << m_taskDeques[idx].size() << std::endl;
                                std::cout << "Thread " << idx << ": doing its job" << std::endl; 
                                currentTask = std::move(m_taskDeques[idx].back());
                                m_taskDeques[idx].pop_back();
                                m_mutexes[idx].unlock();
                            }

                            currentTask();
                        }
                    }
                    
                    // try to steal from someone
                    {
                        for (size_t i = 0; i < MAX_STEAL_ATTEMPTS; ++i)
                        {
                            task_t currentTask;

                            std::this_thread::yield();
                            auto victimThread = GetVictimThread();
                            {
                                m_mutexes[victimThread].lock();
                                if (m_taskDeques[victimThread].empty())
                                {
                                    m_mutexes[victimThread].unlock();
                                    continue;
                                }

                                std::cout << "Thread " << idx << ": stealing from " << victimThread << std::endl; 
                                currentTask = std::move(m_taskDeques[victimThread].front());
                                m_taskDeques[victimThread].pop_front();
                                m_mutexes[victimThread].unlock();
                            }
                            
                            currentTask();
                        }
                    }
                }
            }, i);
    }

    template<class F, class... Args>
    auto Enqueue(F&& f, Args&&... args) 
    {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto currentTask = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        
        std::future<return_type> result = currentTask->get_future();
        {
            auto nextThread = GetNextThread();
            {
                m_mutexes[nextThread].lock();
                m_taskDeques[nextThread].emplace_back([currentTask](){ (*currentTask)(); });
                m_mutexes[nextThread].unlock();
            }
        }

        return result;
    }

    ~ThreadPool()
    {
        m_stop = true;
        for (auto&& worker : m_workers)
            worker.join();
    }
};

}

#endif // #define THREAD_POOL_HEADER
 