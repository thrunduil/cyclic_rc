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

#include "cyclic_rc/config.h"
#include "cyclic_rc/details/ref_count.h"

#include <vector>
#include "boost/smart_ptr/detail/spinlock.hpp"
#include <atomic>

#pragma warning(push)
#pragma warning(disable:4251)

namespace cyclic_rc
{

template<bool multithread>
class cyclic_rc_base;

};

namespace cyclic_rc { namespace details
{

//-------------------------------------------------------------------------
//                      mutex
//-------------------------------------------------------------------------
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

struct nomutex
{
    void lock(){};
    void unlock(){};
};

//-------------------------------------------------------------------------
//                      configs
//-------------------------------------------------------------------------
struct config_nothread
{
    using mutex_type    = nomutex;    
    using atomic_int    = int;

    static const bool is_multithreaded  = false;
};

struct config_thread
{
    using mutex_type    = spinlock;
    using atomic_int    = std::atomic<int>;

    static const bool is_multithreaded  = true;
};

template<bool multithread>
struct make_config{};

template<>
struct make_config<false>
{
    using type  = config_nothread;
};

template<>
struct make_config<true>
{
    using type  = config_thread;
};

//-------------------------------------------------------------------------
//                      obj_count
//-------------------------------------------------------------------------
template<class config>
class collector;    

template<class config>
class obj_count;    

//based on "A Pure Reference Counting Garbage Collector", 
//DAVID F. BACON, CLEMENT R. ATTANASIO, V.T. RAJAN, STEPHEN E. SMITH
template<class config>
class obj_count
{
    private:
        using counter       = details::rc_count;
        using mutex_type    = typename config::mutex_type;
        using atomic_int    = typename config::atomic_int;
        using slot_base     = cyclic_rc_base<config::is_multithreaded>;

	private:
		counter             m_counter;

        CYCLIC_RC_EXPORT
        static mutex_type*  m_mutex;

        friend struct collector_initializer;

	public:
		obj_count(bool is_acyclic);
		~obj_count();

        bool                is_acyclic() const;
        size_t              get_count() const;

        void                increase_refcount();
        static void         decrease_refcount(slot_base* slot);

        template<class T>
        static void         update(T*& old, T* n);

        void                do_visit_children(slot_base* slot, int type);

    public:
        static void         collect(bool all);

	private:        
        void                increase_refcount_impl();
        static void         decrease_refcount_impl(slot_base* s);	

        void                possible_root(slot_base* s);
        void                add_young(slot_base* s);
        void                free_object(slot_base* s);
        static bool         is_freeing();
        void				decrease_refcount_child(slot_base* s);
        void                mark_gray(slot_base* s);                
        void                scan(slot_base* s);	
        void                scan_black(slot_base* s);
        void                collect_white(slot_base* s);        
        void                scan_black_child(slot_base* s);
        void                call_destructor(slot_base* s);
        void				destroy_acyclic(slot_base* s);
        void                release(slot_base* s);

        size_t              get_cout_impl() const;
        bool                is_purple() const;
        bool                is_black() const;
        bool                is_count_zero() const;
        bool                is_young() const;
        bool                is_old() const;
        bool                is_buffered() const;

        void                mark_nonbuffered();
        void                mark_age(details::age_type);

        friend details::collector<config>;
};

};};

#pragma warning(pop)

#include "cyclic_rc/details/obj_count.inl"