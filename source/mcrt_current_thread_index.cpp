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

#include "../include/mcrt_current_thread_index.h"
#include <tbb/task_arena.h>
#include <cassert>

extern "C" uint32_t mcrt_current_thread_index()
{
	// the implementation of parallel map and parallel reduce guarantees all tasks are within the same arena
	int const current_thread_index = tbb::this_task_arena::current_thread_index();
	assert(current_thread_index >= 0);
	return current_thread_index;
}