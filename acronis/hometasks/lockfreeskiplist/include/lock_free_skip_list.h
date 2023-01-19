#ifndef LOCK_FREE_SKIP_LIST
#define LOCK_FREE_SKIP_LIST

#include <array>
#include <limits>
#include <random>
#include <cmath>

#include "tagged_ptr.h"

namespace util
{

namespace detail
{

constexpr unsigned MAX_LEVEL = 5;

class Node
{
    int m_key;
    std::array<TaggedPtr<Node>, MAX_LEVEL + 1> m_next;
    int m_topLevel;
public:
    Node(int key, int height) : m_key(key), m_topLevel(height) {}
    Node(int key) : Node(key, MAX_LEVEL) {}
};

}


template <typename T>
class LockFreeSkipList
{
    detail::Node m_head, m_tail;

    int GetRandomLevel()
    {
        static std::mt19937 gen();
        static std::uniform_real_distribution<double> distribution(0.0, 1.0);

        int level = 1;
        while (distribution(gen) < 0.5f && l < detail::MAX_LEVEL)
            level++;
        return level;
    }
public:
    LockFreeSkipList() : 
        m_head(std::numeric_limits<int>::min()),
        m_tail(std::numeric_limits<int>::max()) 
    {
        for (auto i = 0u; i < m_head.next.length(); ++i)
            m_head.next[i] = TaggedPtr(m_tail);
    }

    bool Add(int x)
    {
        int topLevel = GetRandomLevel();
        int bottomLevel = 0;
        std::array<TaggedPtr<Node>, MAX_LEVEL + 1> preds, succs;

        while (true)
        {
            bool found = Find(x, preds, succs);
            if (found)
            {
                return false;
            }
            else
            {
                Node newNode = 
            }
        }
    }
};


}


#endif // #define LOCK_FREE_SKIP_LIST
