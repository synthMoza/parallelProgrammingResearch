#ifndef EBR_HEADER
#define EBR_HEADER

#include <atomic>
#include <mutex>
#include <list>

#include "exponential_backoff.h"

namespace util
{

namespace detail
{

// number of epochs
constexpr unsigned EPOCHS_COUNT = 3;
// flag that indicates that thread is in critical section
constexpr unsigned ACTIVE_FLAG = 0x80000000U;

// thread local storage for each thread
struct EBR_TLS
{
    std::atomic<unsigned> localEpoch; // this thread local epoch
    EBR_TLS() : localEpoch(0) {}
};

}

class EBR
{
    std::atomic<unsigned> m_globalEpoch;
    std::mutex m_mtx;
    std::list<detail::EBR_TLS*> m_tlsList;

    detail::EBR_TLS* GetEBR_TLS()
    {
        static thread_local detail::EBR_TLS tls;
        return &tls;
    }

public:
    // register a new thread to sync-list
    void Register()
    {
        m_mtx.lock();
        m_tlsList.push_back(GetEBR_TLS());
        m_mtx.unlock();
    }

    // unregister thread (must be called before exit)
    void Unregister()
    {
        m_mtx.lock();
        m_tlsList.remove(GetEBR_TLS());
        m_mtx.unlock();
    }

    // enter "crytical section"
    void Enter()
    {
        auto epoch = m_globalEpoch | detail::ACTIVE_FLAG;
        GetEBR_TLS()->localEpoch.store(epoch, std::memory_order_acquire);
    }

    // leave "crytical section"
    void Exit()
    {
        auto ebr_tls = GetEBR_TLS();
        if (ebr_tls->localEpoch.load(std::memory_order_relaxed) & detail::ACTIVE_FLAG == 0)
            throw std::runtime_error("Attemp to exit from critical section without starting one");
        
        ebr_tls->localEpoch.store(0, std::memory_order_acquire);
    }

    // attempt to syncronize and announce a new epoch
    bool Sync(unsigned* gc_epoch)
    {
        auto epoch = m_globalEpoch.load(std::memory_order_relaxed);
        for (auto& entry : m_tlsList)
        {
            auto localEpoch = entry->localEpoch.load(std::memory_order_release);
            auto isActive = (localEpoch && detail::ACTIVE_FLAG) != 0;

            if (isActive && (localEpoch != (epoch | detail::ACTIVE_FLAG)))
            {
                *gc_epoch = (m_globalEpoch + 1) % detail::EPOCHS_COUNT;
                return false; // can't change epoch now
            }
        }

        // we are here == can change epoch
        m_globalEpoch.store((epoch + 1) % detail::EPOCHS_COUNT, std::memory_order_relaxed);
        *gc_epoch = (m_globalEpoch + 1) % detail::EPOCHS_COUNT;
        
        return true; // successfuly changed epoch
    }

    // full sync between all threads using backoff strategy
    void FullSync()
    {
        ExponentialBackoff backoff{};
        const auto targetEpoch = m_globalEpoch.load(std::memory_order_relaxed);
        auto epoch = 0u;

        do
        {
            while (!Sync(&epoch))
                backoff();
        } while (epoch != targetEpoch);
    }
    
};

}

#endif // #define EBR_HEADER
 