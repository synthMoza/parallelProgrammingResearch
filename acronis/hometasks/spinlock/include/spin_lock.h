#ifndef SPIN_LOCK_HEADER
#define SPIN_LOCK_HEADER

#include <atomic>
#include <thread>

namespace async
{

/*
    Spin lock base class that describes classic interface for it
*/
class ISpinLock
{
public:
    virtual void Lock() = 0;
    virtual void Unlock() = 0;
};

class SpinLockTAS : public ISpinLock
{
    std::atomic<unsigned> m_spin;
public:
    SpinLockTAS() : m_spin(0) {}
    
    void Lock() override
    {
        unsigned expected = 0;
        
        do
        {
            // do not waste cpu time and let threads with same priority run
            std::this_thread::yield();
            expected = 0;
        }
        while (!m_spin.compare_exchange_weak(expected, 1, std::memory_order_acquire, std::memory_order_relaxed));
    }

    void Unlock() override
    {
        m_spin.store(0, std::memory_order_release);
    }
};

class SpinLockTTAS : public ISpinLock
{
    std::atomic<unsigned> m_spin;
public:
    SpinLockTTAS() : m_spin(0) {}
    
    void Lock() override
    {
        unsigned expected = 0;
        
        do
        {
            expected = 0;
         
            // don't invalidate cachelines for other cpu's, read value before trying to win a race between threads to acquire spin
            while (m_spin.load(std::memory_order_relaxed))
                std::this_thread::yield();
        }
        while (!m_spin.compare_exchange_weak(expected, 1, std::memory_order_acquire, std::memory_order_relaxed));
    }

    void Unlock() override
    {
        m_spin.store(0, std::memory_order_release);
    }
};

class SpinLockTicket : public ISpinLock
{
    std::atomic<uint64_t> m_currentTicket;
    std::atomic<uint64_t> m_nextTicket;
public:
    SpinLockTicket() :
        m_currentTicket(0),
        m_nextTicket(0) {}
    
    void Lock() override
    {
        const auto newTicket = m_nextTicket.fetch_add(1, std::memory_order_relaxed);
        while (m_currentTicket.load(std::memory_order_acquire) != newTicket)
            std::this_thread::yield();
    }

    void Unlock() override
    {
        const auto successor = m_currentTicket.load(std::memory_order_relaxed) + 1;
        m_currentTicket.store(successor, std::memory_order_release);
    }

};

}

#endif // #define SPIN_LOCK_HEADER
