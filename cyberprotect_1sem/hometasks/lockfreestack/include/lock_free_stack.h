#ifndef LOCK_FREE_STACK
#define LOCK_FREE_STACK

#include <atomic>
#include <memory>

#include <hazard_ptr.h>

namespace util
{

template <typename T>
class LockFreeStack
{
    struct StackNode
    {
        std::shared_ptr<T> m_data;
        StackNode* m_next;

        StackNode(T const& data) : m_data(std::make_shared<T>(data)) {}  
    };

    std::atomic<StackNode*> m_head;
public:
    LockFreeStack() :
        m_head(nullptr) {}
    
    void push(T const& data)
    {
        ExponentialBackoff backoff{};

        StackNode* const newNode = new StackNode(data);
        newNode->m_next = m_head.load(std::memory_order_acquire);
        while (!m_head.compare_exchange_weak(newNode->m_next, newNode, std::memory_order_release, std::memory_order_relaxed))
            backoff();
    }

    std::shared_ptr<T> pop()
    {
        ExponentialBackoff backoff{};

        std::atomic<void*>& hazardPtr = GetHazardPointerForCurrentThread();
        StackNode* oldHead = m_head.load(std::memory_order_acquire);
        // inner loop - avoid race between acquiring oldHead and setting hazard ptr for it
        // outer loop - change m_head to m_head->next
        do
        {
            StackNode* tmp = nullptr;
            do
            {
                backoff();

                tmp = oldHead;
                hazardPtr.store(oldHead, std::memory_order_acquire);
                oldHead = m_head.load(std::memory_order_acquire);
            } while (oldHead != tmp);
        }
        while (oldHead && !m_head.compare_exchange_strong(oldHead, oldHead->m_next, std::memory_order_release, std::memory_order_relaxed));
        
        //free hazard ptr now
        hazardPtr.store(nullptr, std::memory_order_release);

        std::shared_ptr<T> res;
        if (oldHead)
        {
            res.swap(oldHead->m_data); // do not copy data, just move
            if (OutstandingHazardPointersFor(oldHead)) // if there are hazard ptrs for old head, don't delete it yet
                ReclaimLater(oldHead);
            else
                delete oldHead; // can freely delete it
            
            DeleteNodesWithNoHazards(); // check if there are any hazards we can delete now, if not - next thread will check again
        }

        return res;
    }

    ~LockFreeStack()
    {
        StackNode* current = m_head.load(std::memory_order_relaxed);
        while (current)
        {
            StackNode* oldHead = current;
            current = current->m_next;
            delete oldHead;
        }
    }
};

}

#endif // #define LOCK_FREE_STACK
