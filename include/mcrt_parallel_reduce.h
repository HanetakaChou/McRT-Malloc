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

#ifndef _MCRT_PARALLEL_REDUCE_H_
#define _MCRT_PARALLEL_REDUCE_H_ 1

#include <cstddef>
#include <cstdint>

extern "C" float mcrt_parallel_reduce_float(uint32_t begin, uint32_t end, uint32_t grain_size, float (*serial_reduction)(uint32_t begin, uint32_t end, void *user_data), void *user_data);

extern "C" int32_t mcrt_parallel_reduce_int(uint32_t begin, uint32_t end, uint32_t grain_size, int32_t (*serial_reduction)(uint32_t begin, uint32_t end, void *user_data), void *user_data);

#endif