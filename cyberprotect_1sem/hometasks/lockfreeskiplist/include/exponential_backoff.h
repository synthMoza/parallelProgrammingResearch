#ifndef EXPONENTIAL_BACKOFF_HEADER
#define EXPONENTIAL_BACKOFF_HEADER

#include <stddef.h>

#define ASM_NOP asm volatile ("nop"::);

namespace util
{

class ExponentialBackoff
{
    const size_t m_initialValue;
    const size_t m_step;
    const size_t m_threshold;

    size_t m_current;
public:
    ExponentialBackoff(size_t initialValue = 15, size_t step = 2, size_t threshold = 2048) :
        m_initialValue(initialValue),
        m_step(step),
        m_threshold(threshold),
        m_current(initialValue) {}
    
    void operator()()
    {
        for (size_t i = 0; i < m_current; ++i)
            ASM_NOP;
        
        m_current *= m_step;
        if (m_current > m_threshold)
            m_current = m_threshold;
    }
};

}

#endif // #define EXPONENTIAL_BACKOFF_HEADER
