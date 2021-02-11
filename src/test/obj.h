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

#pragma once 

#include "cyclic_rc/shared_ptr.h"
#include <vector>
#include "boost/smart_ptr/detail/spinlock.hpp"

#pragma warning(push)
#pragma warning(disable: 4127) // conditional expression is constant

namespace cyclic_rc { namespace testing
{

template<bool multithread>
class obj;

template<bool multithread>
using obj_ptr   = cyclic_rc::shared_ptr<class obj<multithread>, multithread>;

class spinlock
{
    private:
        using mutex_type    = boost::detail::spinlock;

    private:
        mutex_type  m_mutex;

    public:
        spinlock()      {m_mutex.v_.clear();};

        void            lock()      { m_mutex.lock(); };
        void            unlock()    { m_mutex.unlock(); };
};

template<bool multi>
struct get_mutex;

template<>
struct get_mutex<true>
{
    using type  = spinlock;
};

template<>
struct get_mutex<false>
{
    struct empty_mutex
    {
        void lock() {};
        void unlock(){};
    };

    using type  = empty_mutex;
};

template<bool multithread>
class obj : public cyclic_rc_base<multithread>
{
    public:
        using obj_vec   = std::vector<obj*>;
        using obj_ptr   = cyclic_rc::shared_ptr<class obj<multithread>, multithread>;

    public:
        obj_ptr         m_left;
        obj_ptr         m_right;        

    public:
        virtual void    visit_children(int op) override;

    private:
        obj(){};

    public:        
        virtual ~obj();

        static obj*         create_obj();
        static void         destroy_obj(obj* ptr);
        
        #if CYCLIC_RC_TEST
            int             m_code;
            static size_t   n_counters();
            void            set_code(int code) { m_code = code; };
        #endif

    private:
        static obj_vec&     get_obj_vec();
};

template<bool multithread>
inline void obj<multithread>::visit_children(int val)
{
    m_left.visit_children(val);
    m_right.visit_children(val);
};

};}

#pragma warning(pop)