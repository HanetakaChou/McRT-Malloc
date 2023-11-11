//
// Copyright (C) YuqiaoZhang(HanetakaChou)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef _MCRT_ALLOCATOR_H_
#define _MCRT_ALLOCATOR_H_ 1

#include "mcrt_malloc.h"

// Apache-2.0 license
// https://github.com/uxlfoundation/oneTBB/blob/master/include/tbb/tbb_allocator.h

#define MCRT_ALLOCATOR_TRAITS_BROKEN 1

#if MCRT_ALLOCATOR_TRAITS_BROKEN
#include <new>
#endif

#include <cstdlib>
#include <utility>

template <typename T>
class mcrt_allocator
{
public:
    using value_type = T;
    using propagate_on_container_move_assignment = std::true_type;

    using is_always_equal = std::true_type;

    inline mcrt_allocator() = default;

    template <typename U>
    inline mcrt_allocator(const mcrt_allocator<U> &) noexcept {}

    inline T *allocate(std::size_t n)
    {
        return static_cast<T *>(mcrt_malloc(n * sizeof(value_type), ((alignof(value_type) > std::size_t(16)) ? alignof(value_type) : std::size_t(16))));
    }

    inline void deallocate(T *p, std::size_t)
    {
        mcrt_free(p);
    }

#if MCRT_ALLOCATOR_TRAITS_BROKEN
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    template <typename U>
    struct rebind
    {
        using other = mcrt_allocator<U>;
    };

    inline size_type max_size() const noexcept
    {
        size_type max = ~(std::size_t(0)) / sizeof(value_type);
        return (max > 0 ? max : 1);
    }

    template <typename U, typename... Args>
    inline void construct(U *p, Args &&...args)
    {
        ::new (p) U(std::forward<Args>(args)...);
    }

    inline void destroy(pointer p)
    {
        p->~value_type();
    }

    inline pointer address(reference x) const
    {
        return &x;
    }

    inline const_pointer address(const_reference x) const
    {
        return &x;
    }
#endif
};

#if MCRT_ALLOCATOR_TRAITS_BROKEN
template <>
class mcrt_allocator<void>
{
public:
    using pointer = void *;
    using const_pointer = const void *;
    using value_type = void;

    template <typename U>
    struct rebind
    {
        using other = mcrt_allocator<U>;
    };
};
#endif

template <typename T, typename U>
inline bool operator==(const mcrt_allocator<T> &, const mcrt_allocator<U> &) noexcept { return true; }

template <typename T, typename U>
inline bool operator!=(const mcrt_allocator<T> &, const mcrt_allocator<U> &) noexcept { return false; }

#endif
