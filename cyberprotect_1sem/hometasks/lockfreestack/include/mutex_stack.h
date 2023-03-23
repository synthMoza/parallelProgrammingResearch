#ifndef MUTEX_STACK_HEADER
#define MUTEX_STACK_HEADER

#include <stack>
#include <mutex>
#include <memory>

namespace util
{

template <typename T>
class MutexStack
{
    struct StackNode
    {
        T m_data;
        StackNode* n_next;

        StackNode(T const& data) : m_data(data) {}  
    };

    StackNode* m_head;
    std::mutex m_mutex;
public:
    MutexStack() :
        m_head(nullptr),
        m_mutex() {}

    void push(T const& data)
    {
        m_mutex.lock();

        auto* newNode = new StackNode(data);
        newNode->n_next = m_head;
        m_head = newNode;

        m_mutex.unlock();
    }

    T pop()
    {
        m_mutex.lock();

        T out{};
        if (m_head)
        {
            auto* oldHead = m_head;
            out = m_head->m_data;
            m_head = m_head->n_next;
            delete oldHead;
        }

        m_mutex.unlock();
        return out;
    }

    ~MutexStack()
    {
        while (m_head)
        {
            auto* oldHead = m_head;
            m_head = m_head->n_next;
            delete oldHead;
        }
    }
};

}

#endif // #define MUTEX_STACK_HEADER
 