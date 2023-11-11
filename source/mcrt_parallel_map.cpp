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

#include "../include/mcrt_parallel_map.h"
#include <tbb/task.h>
#include <cassert>

class task_map : public tbb::task
{
    uint32_t m_begin;
    uint32_t m_end;
    uint32_t const m_grain_size;
    void (*const m_serial_map)(uint32_t begin, uint32_t end, void *user_data);
    void *const m_user_data;

public:
    inline task_map(uint32_t begin, uint32_t end, uint32_t grain_size, void (*serial_map)(uint32_t, uint32_t, void *), void *user_data);

private:
    tbb::task *execute() override;
};

class task_map_continuation : public tbb::task
{
public:
    inline task_map_continuation();

private:
    tbb::task *execute() override;
};

inline task_map::task_map(uint32_t begin, uint32_t end, uint32_t grain_size, void (*serial_map)(uint32_t, uint32_t, void *), void *user_data) : m_begin(begin), m_end(end), m_grain_size(grain_size), m_serial_map(serial_map), m_user_data(user_data)
{
}

tbb::task *task_map::execute()
{
    assert(this->m_end > this->m_begin);
    assert(this->m_grain_size > 0);
    if ((this->m_end - this->m_begin) > this->m_grain_size)
    {
        uint32_t const middle = this->m_begin + (this->m_end - this->m_begin) / 2U;

        // continuation passing style
        task_map_continuation *const task_parent = new (this->allocate_continuation()) task_map_continuation();
        assert(NULL != task_parent);

        task_map *const task_right_child = new (task_parent->allocate_child()) task_map(middle, this->m_end, this->m_grain_size, this->m_serial_map, this->m_user_data);

        assert(NULL != task_right_child);

        // recycle
        this->m_end = middle;
        this->recycle_as_child_of(*task_parent);

        task_parent->set_ref_count(2);

        tbb::task::spawn(*task_right_child);

        // scheduler bypass
        return this;
    }
    else
    {
        this->m_serial_map(this->m_begin, this->m_end, this->m_user_data);
        return NULL;
    }
}

inline task_map_continuation::task_map_continuation()
{
}

tbb::task *task_map_continuation::execute()
{
    return NULL;
}

extern "C" void mcrt_parallel_map(uint32_t begin, uint32_t end, uint32_t grain_size, void (*serial_map)(uint32_t begin, uint32_t end, void *user_data), void *user_data)
{
    task_map *const task_root = new (tbb::task::allocate_root()) task_map(
        begin, end, grain_size,
        serial_map,
        user_data);

    tbb::task::spawn_root_and_wait(*task_root);
}
