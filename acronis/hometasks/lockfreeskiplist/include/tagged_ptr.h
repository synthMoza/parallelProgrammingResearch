#ifndef TAGGED_PTR_HEADER
#define TAGGED_PTR_HEADER

#include <cstddef>
#include <cstdint>

#include "require.h" // check for 64-bit machine

namespace util
{


template <typename T>
class TaggedPtr
{
    T* m_ptr;
public:
    // how many bits are used to mark pointer
    static constexpr uintptr_t TAG_BITS = 16;
    // how many bits are used for actual pointer address
    static constexpr uintptr_t PTR_BITS = 64 - TAG_BITS;
    static constexpr uintptr_t TAG_MASK = ((static_cast<uintptr_t>(1) << (TAG_BITS + 1)) - 1) << PTR_BITS;
    static constexpr uintptr_t PTR_MASK   = ~TAG_MASK;

    TaggedPtr() : m_ptr(nullptr) {}

    TaggedPtr(T* ptr, uintptr_t tag = 0)
        : m_ptr(reinterpret_cast<T*>((PTR_MASK & reinterpret_cast<uintptr_t>(ptr)) | (tag << PTR_BITS))) {}

    uintptr_t GetTag() const noexcept
    {
        return (TAG_MASK & static_cast<uintptr_t>(m_ptr)) >> PTR_BITS;
    }

    T* GetPtr() const noexcept
    {
        return reinterpret_cast<T*>(PTR_MASK & reinterpret_cast<uintptr_t>(m_ptr));
    }

    T& operator*()
    {
        return *GetPtr();
    }

    void Reset() noexcept
    {
        m_ptr = nullptr;
    }

    ~TaggedPtr() {}
};


}

#endif // #define TAGGED_PTR_HEADER
 