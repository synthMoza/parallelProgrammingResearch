#ifndef HAZARD_PTR_HEADER
#define HAZARD_PTR_HEADER

#include <exponential_backoff.h>

#include <atomic>
#include <thread>
#include <array>
#include <functional>
#include <stdexcept>

#include <stddef.h>

namespace util
{

constexpr size_t maxHazardPtrs = 100;

struct HazardPointer
{
    std::atomic<std::thread::id> m_id;
    std::atomic<void*> m_ptr;
};

// list of all hazard pointers at the moment
std::array<HazardPointer, maxHazardPtrs> g_hazardPointers;

class HazardPointerOwner
{
    HazardPointer* m_hp;
public:
    // hazard pointer owner must not be copyable, movable or assignable
    HazardPointerOwner(HazardPointerOwner const&) = delete;
    HazardPointerOwner operator=(HazardPointerOwner const&) = delete;

    HazardPointerOwner() : m_hp(nullptr)
    {
        // find free hazard ptr in global array and claim it
        for (size_t i = 0; i < maxHazardPtrs; ++i)
        {
            std::thread::id oldId;
            if (g_hazardPointers[i].m_id.compare_exchange_strong(oldId, std::this_thread::get_id(), 
                std::memory_order_release, std::memory_order_relaxed))
            {
                // got the id set, now claim it
                m_hp = &g_hazardPointers[i];
                break;
            }
        }

        // haven't found free slot
        if (!m_hp)
            throw std::runtime_error("No free hazard pointer availible");
    }

    std::atomic<void*>& GetPointer()
    {
        return m_hp->m_ptr;
    }

    ~HazardPointerOwner()
    {
        // store nullptr in pointer first, then id
        m_hp->m_ptr.store(nullptr, std::memory_order_relaxed);
        m_hp->m_id.store(std::thread::id{}, std::memory_order_relaxed);
    }
};

std::atomic<void*>& GetHazardPointerForCurrentThread()
{
    thread_local static HazardPointerOwner hazard; // each threads constructs owner for itself as thread_local static variable
    return hazard.GetPointer();
}

bool OutstandingHazardPointersFor(void* p)
{
    for (size_t i = 0; i < maxHazardPtrs; ++i)
        if (g_hazardPointers[i].m_ptr.load(std::memory_order_release) == p)
            return true;
    
    return false;
}

template <typename T>
void DoDelete(void* p)
{
    delete static_cast<T*>(p);
}

struct DataToReclaim
{
    void* m_data;
    std::function<void(void*)> m_deleter;
    DataToReclaim* m_next;

    template <typename T>
    DataToReclaim(T* p) :
        m_data(p),
        m_deleter(&DoDelete<T>),
        m_next(nullptr) {}
    
    ~DataToReclaim()
    {
        m_deleter(m_data);
    }
};

// reclaim list
std::atomic<DataToReclaim*> g_nodesToReclaim;

void AddToReclaimList(DataToReclaim* node)
{
    ExponentialBackoff backoff;

    node->m_next = g_nodesToReclaim.load(std::memory_order_acquire);
    while (!g_nodesToReclaim.compare_exchange_weak(node->m_next, node))
        backoff();
}

template <typename T>
void ReclaimLater(T* data)
{
    AddToReclaimList(new DataToReclaim(data));
}

void DeleteNodesWithNoHazards()
{
    // claim whole list
    DataToReclaim* current = g_nodesToReclaim.exchange(nullptr, std::memory_order_relaxed);
    while (current)
    {
        DataToReclaim* const next = current->m_next;
        if (!OutstandingHazardPointersFor(current->m_data))
        {
            delete current;
        }
        else
        {
            AddToReclaimList(current);
        }

        current = next;
    }
}

}

#endif // #define HAZARD_PTR_HEADER
 