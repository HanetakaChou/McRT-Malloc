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

#include "../include/mcrt_parallel_reduce.h"
#include <tbb/task.h>
#include <cassert>

template <typename T>
class task_reduction : public tbb::task
{
    T *m_reduction_parent;
    T const m_reduction_zero;
    T (*const m_reduction_operator)(T const &left, T const &right);
    uint32_t m_begin;
    uint32_t m_end;
    uint32_t const m_grain_size;
    T (*const m_serial_reduction)(uint32_t begin, uint32_t end, void *user_data);
    void *const m_user_data;

public:
    inline task_reduction(T *reduction_parent, T const &reduction_zero, T (*const reduction_operator)(T const &, T const &), uint32_t begin, uint32_t end, uint32_t grain_size, T (*serial_reduction)(uint32_t, uint32_t, void *), void *user_data);

private:
    tbb::task *execute() override;
};

template <typename T>
class task_reduction_continuation : public tbb::task
{
    T *m_reduction_parent;
    T (*const m_reduction_operator)(T const &, T const &);
    T m_reduction_left_child;
    T m_reduction_right_child;

public:
    inline task_reduction_continuation(T **inout_reduction_parent, T const &reduction_zero, T (*const reduction_operator)(T const &, T const &));

    inline task_reduction<T> *allocate_right_child(T const &reduction_zero, T (*const reduction_operator)(T const &, T const &), uint32_t begin, uint32_t end, uint32_t grain_size, T (*serial_reduction)(uint32_t, uint32_t, void *), void *user_data);

private:
    tbb::task *execute() override;
};

template <typename T>
inline task_reduction<T>::task_reduction(T *reduction_parent, T const &reduction_zero, T (*const reduction_operator)(T const &, T const &), uint32_t begin, uint32_t end, uint32_t grain_size, T (*serial_reduction)(uint32_t, uint32_t, void *), void *user_data) : m_reduction_parent(reduction_parent), m_reduction_zero(reduction_zero), m_reduction_operator(reduction_operator), m_begin(begin), m_end(end), m_grain_size(grain_size), m_serial_reduction(serial_reduction), m_user_data(user_data)
{
}

template <typename T>
tbb::task *task_reduction<T>::execute()
{
    assert(this->m_end > this->m_begin);
    assert(this->m_grain_size > 0);
    if ((this->m_end - this->m_begin) > this->m_grain_size)
    {
        uint32_t const middle = this->m_begin + (this->m_end - this->m_begin) / 2U;

        // continuation passing style
        task_reduction_continuation<T> *const task_parent = new (this->allocate_continuation()) task_reduction_continuation<T>(&this->m_reduction_parent, this->m_reduction_zero, this->m_reduction_operator);
        assert(NULL != task_parent);

        task_reduction *const task_right_child = task_parent->allocate_right_child(this->m_reduction_zero, this->m_reduction_operator, middle, this->m_end, this->m_grain_size, this->m_serial_reduction, this->m_user_data);
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
        (*this->m_reduction_parent) = this->m_serial_reduction(this->m_begin, this->m_end, this->m_user_data);
        return NULL;
    }
}

template <typename T>
inline task_reduction_continuation<T>::task_reduction_continuation(T **inout_reduction_parent, T const &reduction_zero, T (*const reduction_operator)(T const &, T const &)) : m_reduction_parent(*inout_reduction_parent), m_reduction_operator(reduction_operator), m_reduction_left_child(reduction_zero), m_reduction_right_child(reduction_zero)
{
    (*inout_reduction_parent) = &this->m_reduction_left_child;
}

template <typename T>
inline task_reduction<T> *task_reduction_continuation<T>::allocate_right_child(T const &reduction_zero, T (*const reduction_operator)(T const &, T const &), uint32_t begin, uint32_t end, uint32_t grain_size, T (*serial_reduction)(uint32_t, uint32_t, void *), void *user_data)
{
    task_reduction<T> *task_right = new (this->allocate_child()) task_reduction<T>(&this->m_reduction_right_child, reduction_zero, reduction_operator, begin, end, grain_size, serial_reduction, user_data);

    return task_right;
}

template <typename T>
tbb::task *task_reduction_continuation<T>::execute()
{
    (*this->m_reduction_parent) = this->m_reduction_operator(this->m_reduction_left_child, this->m_reduction_right_child);
    return NULL;
}

extern "C" float mcrt_parallel_reduce_float(uint32_t begin, uint32_t end, uint32_t grain_size, float (*serial_reduction)(uint32_t begin, uint32_t end, void *user_data), void *user_data)
{
    float reduction = 0.0F;

    task_reduction<float> *const task_root = new (tbb::task::allocate_root()) task_reduction<float>(
        &reduction,
        0.0F,
        [](float const &left, float const &right) -> float
        { return left + right; },
        begin, end, grain_size,
        serial_reduction,
        user_data);

    tbb::task::spawn_root_and_wait(*task_root);

    return reduction;
}

extern "C" double mcrt_parallel_reduce_double(uint32_t begin, uint32_t end, uint32_t grain_size, double (*serial_reduction)(uint32_t begin, uint32_t end, void *user_data), void *user_data)
{
    double reduction = 0.0;

    task_reduction<double> *const task_root = new (tbb::task::allocate_root()) task_reduction<double>(
        &reduction,
        0.0,
        [](double const &left, double const &right) -> double
        { return left + right; },
        begin, end, grain_size,
        serial_reduction,
        user_data);

    tbb::task::spawn_root_and_wait(*task_root);

    return reduction;
}

extern "C" mcrt_double2 mcrt_parallel_reduce_double2(uint32_t begin, uint32_t end, uint32_t grain_size, mcrt_double2 (*serial_reduction)(uint32_t begin, uint32_t end, void *user_data), void *user_data)
{
    mcrt_double2 reduction = {0.0, 0.0};

    task_reduction<mcrt_double2> *const task_root = new (tbb::task::allocate_root()) task_reduction<mcrt_double2>(
        &reduction,
        mcrt_double2{0.0, 0.0},
        [](mcrt_double2 const &left, mcrt_double2 const &right) -> mcrt_double2
        { return mcrt_double2{left.x + right.x, left.y + right.y}; },
        begin, end, grain_size,
        serial_reduction,
        user_data);

    tbb::task::spawn_root_and_wait(*task_root);

    return reduction;
}

extern "C" mcrt_double4 mcrt_parallel_reduce_double4(uint32_t begin, uint32_t end, uint32_t grain_size, mcrt_double4 (*serial_reduction)(uint32_t begin, uint32_t end, void *user_data), void *user_data)
{
    mcrt_double4 reduction = {0.0, 0.0, 0.0, 0.0};

    task_reduction<mcrt_double4> *const task_root = new (tbb::task::allocate_root()) task_reduction<mcrt_double4>(
        &reduction,
        mcrt_double4{0.0, 0.0, 0.0, 0.0},
        [](mcrt_double4 const &left, mcrt_double4 const &right) -> mcrt_double4
        { return mcrt_double4{left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w}; },
        begin, end, grain_size,
        serial_reduction,
        user_data);

    tbb::task::spawn_root_and_wait(*task_root);

    return reduction;
}

extern "C" int32_t mcrt_parallel_reduce_int(uint32_t begin, uint32_t end, uint32_t grain_size, int32_t (*serial_reduction)(uint32_t begin, uint32_t end, void *user_data), void *user_data)
{
    int32_t reduction = 0;

    task_reduction<int32_t> *const task_root = new (tbb::task::allocate_root()) task_reduction<int32_t>(
        &reduction,
        0,
        [](int32_t const &left, int32_t const &right) -> int32_t
        { return left + right; },
        begin, end, grain_size,
        serial_reduction,
        user_data);

    tbb::task::spawn_root_and_wait(*task_root);

    return reduction;
}