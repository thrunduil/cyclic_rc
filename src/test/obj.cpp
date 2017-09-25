/* 
 *  This file is a part of cyclic_rc library.
 *
 *  Copyright (c) Pawe³ Kowal 2017
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

#pragma once 

#include "obj.h"
#include "boost/smart_ptr/detail/spinlock.hpp"
#include <mutex>
#include <set>

#define DEBUG_RC 0

namespace cyclic_rc { namespace testing
{

std::atomic<int> g_code = 0;
std::atomic<int> g_count = 0;

template<bool multithread>
thread_local static
typename obj<multithread>::obj_vec* pool_obj = nullptr;

template<bool multithread>
thread_local static
typename obj<multithread>::count_vec* 
pool_count = nullptr;

template<bool multithread>
typename obj<multithread>::obj_vec& 
obj<multithread>::get_obj_vec()
{
    if (pool_obj<multithread> == nullptr)
        pool_obj<multithread> = new obj_vec();

    return *pool_obj<multithread>;
};

template<bool multithread>
struct leak_detector
{    
    using obj       = obj<multithread>;
    using obj_set   = std::set<obj*>;
    using mutex     = typename get_mutex<multithread>::type;

    static obj_set  m_set;
    static mutex    m_mutex;

    static void     report_new(obj* obj);
    static void     report_delete(obj* obj);

    static const obj_set&     get_set();
};

template<bool multithread>
typename leak_detector<multithread>::obj_set 
leak_detector<multithread>::m_set;

template<bool multithread>
typename leak_detector<multithread>::mutex 
leak_detector<multithread>::m_mutex;

template<bool multithread>
void leak_detector<multithread>::report_new(obj* obj)
{
    std::unique_lock<mutex> lock(m_mutex);
    auto pos = m_set.find(obj);

    if (pos != m_set.end())
        throw;

    m_set.insert(pos, obj);
}

template<bool multithread>
void leak_detector<multithread>::report_delete(obj* obj)
{
    std::unique_lock<mutex> lock(m_mutex);
    auto pos = m_set.find(obj);

    if (pos == m_set.end())
        throw;

    m_set.erase(pos);
}

template<bool multithread>
const typename leak_detector<multithread>::obj_set& 
leak_detector<multithread>::get_set()
{
    return m_set;
}

template<bool multithread>
obj<multithread>* obj<multithread>::create_obj()
{    
    obj* ret;

    {
        obj_vec& pool = get_obj_vec();

        if (pool.size() > 0)
        {
            obj* ptr = pool.back();
            pool.pop_back();       
            ret = ptr;
        }
        else
        {
            ret = nullptr;
        };
    };

    if (ret == nullptr)
        ret = new obj();
    else
        new(ret) obj();

    #if DEBUG_RC
        ret->set_code(g_code);
        ++g_code;
        ++g_count;
        leak_detector<multithread>::report_new(ret);
    #endif    

    return ret;
};

template<bool multithread>
void obj<multithread>::destroy_obj(obj* ptr)
{
    //delete ptr;

    #if DEBUG_RC
        leak_detector<multithread>::report_delete(ptr);
        --g_count;
    #endif

    obj_vec& pool = get_obj_vec();       
    pool.push_back(ptr);    
};

template<bool multithread>
obj<multithread>::~obj()
{
    destroy_obj(this);
};


#if CYCLIC_RC_TEST
    template<bool multithread>
    size_t obj<multithread>::n_counters()
    {
        const auto& s = leak_detector<multithread>::get_set();
        (void)s;
        size_t count = g_count;
        return count;
    };
#endif

template class obj<false>;
template class obj<true>;

};}