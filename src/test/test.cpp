/* 
 *  This file is a part of cyclic_rc library.
 *
 *  Copyright (c) Pawe³ Kowal 2017 - 2021
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "test.h"
#include <iostream>
#include <mutex>

namespace cyclic_rc { namespace testing
{

template<class Base>
struct derived_type : Base
{};

template<bool multithread>
class obj2 : public cyclic_rc_base<multithread>
{
    public:
        virtual void visit_children(int op) override
        {
            (void)op;
        };
};


template <bool multithread>
void test_compile()
{    
    using obj           = obj< multithread>;
    using obj2          = obj2< multithread>;
    using obj_der       = derived_type<obj>;

    using obj_ptr       = obj_ptr< multithread>;    
    using obj_der_ptr   = shared_ptr<obj_der>;
    using obj2_ptr      = shared_ptr<obj2>;

    obj* ptr            = obj::create_obj();
    obj_der* ptr_d      = (obj_der*)obj_der::create_obj();

    {
        obj_ptr tmp1;
        obj_ptr tmp2(nullptr);
        obj_ptr tmp3(ptr);
        obj_ptr tmp32(ptr_d);

        obj_ptr tmp4(tmp1);
        obj_ptr tmp5(std::move(tmp4));

        tmp5    = tmp1;
        tmp5    = std::move(tmp1);

        tmp5.reset();
        tmp5.reset(ptr);

        tmp5.swap(tmp2);
        swap(tmp5, tmp2);

        bool val1   = (bool)tmp5;
        bool val2   = !tmp5;

        (void)val1;
        (void)val2;

        obj* ptr2   = tmp2.get();
        obj& ref1   = *tmp3;

        (void)ptr2;
        (void)ref1;

        tmp3->m_left    = tmp2;

        bool is_unique  = tmp2.unique();
        size_t count    = tmp2.use_count();

        (void)is_unique;
        (void)count;        
    }

    {
        obj_ptr         p1(ptr);
        obj_der_ptr     p2(ptr_d);

        obj_ptr p3(p2);
        obj_ptr p4(std::move(p2));

        p3      = p2;
        p4      = std::move(p2);
    };

    obj_ptr::collect(true);
};

template <bool multithread>
typename test<multithread>::obj_vector 
test<multithread>::m_obj_vector_global;

template <bool multithread>
typename test<multithread>::mutex_type* 
test<multithread>::m_global_mutex = new mutex_type();

template <bool multithread>
void test<multithread>::make(int n_operations)
{
    test_compile< multithread>();

    for (int i = 0; i < n_operations; ++i)
    {
        if (i % 10000000 == 0 && i > 0)
            std::cout << i << "\n";

        operation_type op = rand_op();

        switch (op)
        {
            case operation_type::create_new:
                op_create_new();
                break;
            case operation_type::destroy_existing:
                op_destroy_existing();
                break;
            case operation_type::assign_left_new:
                op_assign_left_new();
                break;
            case operation_type::assign_right_new:
                op_assign_right_new();
                break;
            case operation_type::assign_left_existing:
                op_assign_left_existing();
                break;
            case operation_type::assign_right_existing:
                op_assign_right_existing();
                break;
            case operation_type::destroy_left:
                op_destroy_left();
                break;
            case operation_type::destroy_right:
                op_destroy_right();
                break;
            case operation_type::change:
                op_change();
                break;
            case operation_type::store_global:
                op_store_global();
                break;
            case operation_type::load_global:
                op_load_global();
                break;
            case operation_type::delete_global:
                op_delete_global();
            default:
                break;
        };

        if (make_clear_all() == true)
            op_clear_all();
    };
};

template <bool multithread>
bool test<multithread>::make_clear_all()
{
    if (rand() % N == 0)
        return true;
    else
        return false;
};

template <bool multithread>
typename test<multithread>::operation_type 
test<multithread>::rand_op()
{
    int pos = rand() % ((int)operation_type::last - 1);

    return static_cast<operation_type>(pos);
};

template <bool multithread>
int test<multithread>::rand_pos()
{
    size_t s = m_obj_vector.size();

    if (s == 0)
        return -1;

    s = s-1;

    if (s == 0)
        return 0;

    int pos = rand() % s;

    return pos;
};

template <bool multithread>
int test<multithread>::rand_global_pos()
{
    size_t s = m_obj_vector_global.size();

    if (s == 0)
        return -1;

    s = s-1;

    if (s == 0)
        return 0;

    int pos = rand() % s;

    return pos;
};

template <bool multithread>
void test<multithread>::op_create_new()
{
    obj_ptr op(obj::create_obj());
    m_obj_vector.push_back(op);
};

template <bool multithread>
bool test<multithread>::do_global() const
{
    if (rand() < 1e-3)
        return true;
    else
        return false;
};

template <bool multithread>
void test<multithread>::op_store_global()
{
    if (do_global() == false)
        return;

    if (m_obj_vector.size() == 0)
        op_create_new();

    int pos             = rand_pos();
    obj_ptr elem        = m_obj_vector[pos];

    this->store_global(elem);
};

template <bool multithread>
void test<multithread>::op_load_global()
{
    if (do_global() == false)
        return;

    obj_ptr elem        = this->load_global();
    m_obj_vector.push_back(elem);
};

template <bool multithread>
void test<multithread>::op_destroy_existing()
{
    if (m_obj_vector.size() == 0)
        return;

    int pos             = rand_pos();
    m_obj_vector[pos]   = m_obj_vector.back();

    m_obj_vector.pop_back();
};

template <bool multithread>
void test<multithread>::op_assign_left_new()
{
    if (m_obj_vector.size() == 0)
        op_create_new();

    obj_ptr op(obj::create_obj());

    int pos                     = rand_pos();
    m_obj_vector[pos]->m_left   = op;
};

template <bool multithread>
void test<multithread>::op_assign_right_new()
{
    if (m_obj_vector.size() == 0)
        op_create_new();

    obj_ptr op(obj::create_obj());

    int pos                     = rand_pos();
    m_obj_vector[pos]->m_right  = op;
};

template <bool multithread>
void test<multithread>::op_assign_left_existing()
{
    if (m_obj_vector.size() == 0)
        op_create_new();

    int pos1                    = rand_pos();
    int pos2                    = rand_pos();
    m_obj_vector[pos1]->m_left  = m_obj_vector[pos2];
};

template <bool multithread>
void test<multithread>::op_assign_right_existing()
{
    if (m_obj_vector.size() == 0)
        op_create_new();

    int pos1                    = rand_pos();
    int pos2                    = rand_pos();
    m_obj_vector[pos1]->m_right = m_obj_vector[pos2];
};

template <bool multithread>
void test<multithread>::op_destroy_left()
{
    if (m_obj_vector.size() == 0)
        return;

    int pos1                    = rand_pos();
    m_obj_vector[pos1]->m_left  = obj_ptr();
};

template <bool multithread>
void test<multithread>::op_destroy_right()
{
    if (m_obj_vector.size() == 0)
        return;

    int pos1                    = rand_pos();
    m_obj_vector[pos1]->m_right = obj_ptr();
};

template <bool multithread>
void test<multithread>::op_change()
{
    if (m_obj_vector.size() == 0)
        return;

    obj_ptr op(obj::create_obj());

    int pos1                    = rand_pos();
    m_obj_vector[pos1]          = op;
}

template <bool multithread>
void test<multithread>::store_global(const obj_ptr& obj)
{
    std::lock_guard<mutex_type> lock(*m_global_mutex);
    m_obj_vector_global.push_back(obj);
};

template <bool multithread>
typename test<multithread>::obj_ptr 
test<multithread>::load_global()    
{
    std::lock_guard<mutex_type> lock(*m_global_mutex);

    if (m_obj_vector_global.size() == 0)
    {
        obj_ptr op(obj::create_obj());
        m_obj_vector_global.push_back(op);
    };

    int pos1 = rand_global_pos();

    return m_obj_vector_global[pos1];
};

template <bool multithread>
void test<multithread>::op_delete_global()    
{
    if (do_global() == false)
        return;

    std::lock_guard<mutex_type> lock(*m_global_mutex);

    if (m_obj_vector_global.size() == 0)
    {
        return;
    };

    int pos = rand_global_pos();

    m_obj_vector_global[pos]   = m_obj_vector_global.back();

    m_obj_vector_global.pop_back();
};

template <bool multithread>
void test<multithread>::clear_global()
{
    std::lock_guard<mutex_type> lock(*m_global_mutex);
    m_obj_vector_global.clear();
};

template <bool multithread>
void test<multithread>::op_clear_all()
{
    m_obj_vector.clear();

    obj_ptr::collect(true);
};

template class test<false>;
template class test<true>;

};}