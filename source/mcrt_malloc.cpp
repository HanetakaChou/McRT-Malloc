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

#include "../include/mcrt_malloc.h"
#include <tbb/scalable_allocator.h>
#include <cstring>

extern "C" void *mcrt_malloc(size_t size, size_t alignment)
{
	void *ptr = scalable_aligned_malloc(size, alignment);
#ifndef NDEBUG
	std::memset(ptr, 0XCDCDCDCD, size);
#endif
	return ptr;
}

extern "C" void mcrt_free(void *ptr)
{
	scalable_aligned_free(ptr);
}

extern "C" void *mcrt_realloc(void *old_ptr, size_t size, size_t alignment)
{
#ifndef NDEBUG
	size_t old_size = scalable_msize(old_ptr);
#endif
	void *new_ptr = scalable_aligned_realloc(old_ptr, size, alignment);
#ifndef NDEBUG
	if (size > old_size)
	{
		std::memset(static_cast<uint8_t *>(new_ptr) + old_size, 0XCDCDCDCD, size - old_size);
	}
#endif
	return new_ptr;
}
