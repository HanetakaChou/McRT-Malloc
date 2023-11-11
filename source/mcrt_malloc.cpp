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
#include <stdlib.h>
#include <memory.h>

// TODO: implement McRT-Malloc

extern void *mcrt_malloc(size_t raw_size, size_t alignment)
{
	// [alignUp](https://github.com/oneapi-src/oneTBB/blob/tbb_2019/src/tbbmalloc/shared_utils.h#L42)
	size_t const size = ((((static_cast<size_t>(sizeof(float)) * raw_size) - static_cast<size_t>(1U)) | (alignment - static_cast<size_t>(1U))) + static_cast<size_t>(1U));

#if defined(__GNUC__)
	return aligned_alloc(alignment, size);
#elif defined(_MSC_VER)
	return _aligned_malloc(size, alignment);
#else
#error Unknown Compiler
#endif
}

extern void mcrt_free(void *ptr)
{
#if defined(__GNUC__)
	free(ptr);
#elif defined(_MSC_VER)
	_aligned_free(ptr);
#else
#error Unknown Compiler
#endif
}
