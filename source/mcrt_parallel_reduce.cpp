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

extern "C" mcrt_double36 mcrt_parallel_reduce_double36(uint32_t begin, uint32_t end, uint32_t grain_size, mcrt_double36 (*serial_reduction)(uint32_t begin, uint32_t end, void *user_data), void *user_data)
{
    mcrt_double36 reduction = {
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    task_reduction<mcrt_double36> *const task_root = new (tbb::task::allocate_root()) task_reduction<mcrt_double36>(
        &reduction,
        mcrt_double36{
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        [](mcrt_double36 const &left, mcrt_double36 const &right) -> mcrt_double36
        {
            return mcrt_double36{
                left.v[0] + right.v[0], left.v[1] + right.v[1], left.v[2] + right.v[2], left.v[3] + right.v[3], left.v[4] + right.v[4], left.v[5] + right.v[5],
                left.v[6] + right.v[6], left.v[7] + right.v[7], left.v[8] + right.v[8], left.v[9] + right.v[9], left.v[10] + right.v[10], left.v[11] + right.v[11],
                left.v[12] + right.v[12], left.v[13] + right.v[13], left.v[14] + right.v[14], left.v[15] + right.v[15], left.v[16] + right.v[16], left.v[17] + right.v[17],
                left.v[18] + right.v[18], left.v[19] + right.v[19], left.v[20] + right.v[20], left.v[21] + right.v[21], left.v[22] + right.v[22], left.v[23] + right.v[23],
                left.v[24] + right.v[24], left.v[25] + right.v[25], left.v[26] + right.v[26], left.v[27] + right.v[27], left.v[28] + right.v[28], left.v[29] + right.v[29],
                left.v[30] + right.v[30], left.v[31] + right.v[31], left.v[32] + right.v[32], left.v[33] + right.v[33], left.v[34] + right.v[34], left.v[35] + right.v[35]};
        },
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