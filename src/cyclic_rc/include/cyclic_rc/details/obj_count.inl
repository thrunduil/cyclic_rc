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

#include "cyclic_rc/details/obj_count.h"
#include "cyclic_rc/details/collector.h"

#include <cassert>
#include <mutex>

namespace cyclic_rc { namespace details
{

enum class collect_type : int
{
    decrease_ref, decrease_ref_test, scan,  collect_white, scan_black
};

//-------------------------------------------------------------------------
//                      obj_count
//-------------------------------------------------------------------------
template<class config>
CYCLIC_RC_FORCE_INLINE
bool obj_count<config>::is_acyclic() const
{
    return m_counter.is_acyclic();
};

template<class config>
CYCLIC_RC_FORCE_INLINE
void obj_count<config>::increase_refcount()
{
    std::lock_guard<mutex_type> lock(*m_mutex);

	increase_refcount_impl();
};

template<class config>
CYCLIC_RC_FORCE_INLINE
void obj_count<config>::increase_refcount_impl()
{
	m_counter.increase_count();
    m_counter.mark_black();
};

template<class config>
CYCLIC_RC_FORCE_INLINE
size_t obj_count<config>::get_count() const
{
    std::lock_guard<mutex_type> lock(*m_mutex);

    return m_counter.get_count();
};

template<class config>
CYCLIC_RC_FORCE_INLINE
size_t obj_count<config>::get_cout_impl() const
{
    return m_counter.get_count();
};

template<class config>
CYCLIC_RC_FORCE_INLINE
bool obj_count<config>::is_count_zero() const
{
	return m_counter.is_count_zero();
};

template<class config>
CYCLIC_RC_FORCE_INLINE
bool obj_count<config>::is_young() const
{
    return m_counter.is_young();
};

template<class config>
CYCLIC_RC_FORCE_INLINE
bool obj_count<config>::is_old() const
{
    return m_counter.is_old();
};

template<class config>
CYCLIC_RC_FORCE_INLINE
bool obj_count<config>::is_buffered() const
{
    return m_counter.is_buffered();
};

template<class config>
CYCLIC_RC_FORCE_INLINE
bool obj_count<config>::is_purple() const
{
	return m_counter.is_purple();
};

template<class config>
CYCLIC_RC_FORCE_INLINE
void obj_count<config>::mark_nonbuffered()
{
	return m_counter.mark_nonbuffered();
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::mark_age(details::age_type age)
{
    m_counter.mark_age(age);
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
bool obj_count<config>::is_black() const
{
	return m_counter.is_black();
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::destroy_acyclic(slot_base* s)
{
	call_destructor(s);
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::call_destructor(slot_base* s)
{
    s->~slot_base();
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::mark_gray(slot_base* s)
{
	if (m_counter.is_gray() == false)
	{
		m_counter.mark_gray();
		s->visit_children((int)collect_type::decrease_ref_test);
	};
}

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::scan_black(slot_base* s)
{
	m_counter.mark_black();
	s->visit_children((int)collect_type::scan_black);
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::scan_black_child(slot_base* s)
{
    m_counter.increase_count();

	if (m_counter.is_black() == false)
		scan_black(s);
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::scan(slot_base* s)
{
	if (m_counter.is_gray())
	{
		if (m_counter.is_count_zero() == false)
		{
			scan_black(s);
		}
		else
		{
			m_counter.mark_white();
			s->visit_children((int)collect_type::scan);
		}				
	};
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::decrease_refcount_child(slot_base* s)
{
    if (m_counter.decrease_count() == 0)
        return release(s);
    else if (m_counter.is_acyclic() == false)
        return possible_root(s);
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::possible_root(slot_base* s)
{
    if (this->is_purple() == false)
	{
        m_counter.mark_purple();

        if (m_counter.is_young() == false)
		{
            m_counter.mark_buffered();
            m_counter.mark_age(details::age_type::young);
			add_young(s);
		}
	};
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::collect_white(slot_base* s)
{
	if (m_counter.is_white())
	{
        if (m_counter.is_old() == false)
        {
            m_counter.mark_nonbuffered();
        };
        if (m_counter.is_buffered() == false)
        {
		    m_counter.mark_black();
		
            s->visit_children((int)collect_type::collect_white);

            free_object(s);
        };
	};
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::release(slot_base* s)
{
    if (m_counter.is_acyclic() == true)
	{
        this->destroy_acyclic(s);
		return;
	};

	s->visit_children((int)collect_type::decrease_ref);
    m_counter.mark_black();

    if (m_counter.is_buffered() == false)
        free_object(s);
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::decrease_refcount(slot_base* s)
{    
    // if is_freeing == true, then collector is calling destructors
    // of garbage objects, which calls destructors of children; however
    // these destructors are already called (implicitly), refcounts are
    // updated, therefore we must return immediately;
    // however in other threads destructors must be processed;
    // is_freeing is thread local, therefore properly identifies, this two
    // cases

    if (is_freeing() == true)
        return;

    std::lock_guard<mutex_type> lock(*m_mutex);

    decrease_refcount_impl(s);
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::decrease_refcount_impl(slot_base* s)
{    
    if (s->get_counter().m_counter.decrease_count() == 0)
        return s->get_counter().release(s);
    else if (s->get_counter().m_counter.is_acyclic() == false)
        return s->get_counter().possible_root(s);
};

template<class config>
template<class T>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::update(T*& old, T* n)
{
    std::lock_guard<mutex_type> lock(*m_mutex);

	if(n != nullptr)
        n->get_counter().increase_refcount_impl();
    
    slot_base* o = old;
    old         = n;

    if (o != nullptr)
        obj_count::decrease_refcount_impl(o);
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
obj_count<config>::obj_count(bool is_acyclic)
    :m_counter(is_acyclic)
{};

template<class config>
CYCLIC_RC_FORCE_INLINE 
obj_count<config>::~obj_count()
{};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::collect(bool all)
{
    std::lock_guard<mutex_type> lock(*m_mutex);
    details::collector<config>::make_collect(all);
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::add_young(slot_base* s)
{
    details::collector<config>::add_young(s);
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
void obj_count<config>::free_object(slot_base* s)
{
    details::collector<config>::free_object(s);
};

template<class config>
CYCLIC_RC_FORCE_INLINE 
bool obj_count<config>::is_freeing()
{
    return details::collector<config>::is_freeing();
};

template<class config>
void obj_count<config>::do_visit_children(slot_base* s, int type)
{
	switch(type)
	{
        case collect_type::decrease_ref:
		{
			this->decrease_refcount_child(s);
			break;
		}
        case collect_type::decrease_ref_test:
		{
			m_counter.decrease_count();
			this->mark_gray(s);
			break;
		}
		case collect_type::scan:
		{
			this->scan(s);
			break;
		}
        case collect_type::scan_black:
		{
			this->scan_black_child(s);
			break;
		}
        case collect_type::collect_white:
		{
			this->collect_white(s);
			break;
		}
	};
};

};};