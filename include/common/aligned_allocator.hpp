#pragma once

#include <vector>
#include <xmmintrin.h>

namespace plane_render {

template<typename T, std::size_t align>
struct AlignmentAllocator
{
public:
    using value_type = T;

    template<typename U>
    struct rebind
    {
        using other = AlignmentAllocator<U, align>;
    };

public:
    AlignmentAllocator()
    {}

    template <typename T2>
    AlignmentAllocator(const AlignmentAllocator<T2, align>&)
    {}

    T* allocate(std::size_t n)
    {
        return static_cast<T*>(_mm_malloc(n * sizeof(T), align));
    }

    void deallocate(T* const ptr, std::size_t)
    {
        _mm_free(ptr);
    }
};

template<typename T>
using VectorAlignment16 = std::vector<T, AlignmentAllocator<T, 16>>;

} // namespace plane_render