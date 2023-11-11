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

#include "../include/mcrt_max_concurrency.h"
#include <tbb/task_arena.h>
#include <cassert>

extern "C" uint32_t mcrt_max_concurrency()
{
	int const max_concurrency = tbb::this_task_arena::max_concurrency();
	assert(max_concurrency > 0);
	return max_concurrency;
}