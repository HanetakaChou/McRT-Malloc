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

#ifndef _MCRT_UNORDERED_MAP_H_
#define _MCRT_UNORDERED_MAP_H_ 1

#include <unordered_map>
#include "mcrt_allocator.h"

template <typename Key, typename T>
using mcrt_unordered_map = std::unordered_map<Key, T, std::hash<Key>, std::equal_to<Key>, mcrt_allocator<std::pair<const Key, T>>>;

#endif
