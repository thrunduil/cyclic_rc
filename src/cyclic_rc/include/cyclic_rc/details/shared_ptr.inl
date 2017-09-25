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

#include "cyclic_rc/shared_ptr.h"

namespace cyclic_rc
{

template<bool multithread>
CYCLIC_RC_FORCE_INLINE
cyclic_rc_base<multithread>::cyclic_rc_base(bool is_acyclic)
    :m_counter(is_acyclic)
{};

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
void shared_ptr<T, multithread>::init()
{           
    if (m_ptr)
		m_ptr->get_counter().increase_refcount();
}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
shared_ptr<T, multithread>::shared_ptr()
: m_ptr()
{}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
shared_ptr<T, multithread>::shared_ptr(nullptr_t)
: m_ptr()
{}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
shared_ptr<T, multithread>::shared_ptr(pointer_type obj)
: m_ptr(obj)
{
	init();
}

template<typename T, bool multithread>
template<class U>
CYCLIC_RC_FORCE_INLINE
shared_ptr<T, multithread>::shared_ptr(U* obj)
: m_ptr(obj)
{
	init();
}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
shared_ptr<T, multithread>::shared_ptr(const shared_ptr& rhs)
: m_ptr(rhs.m_ptr)
{
	init();
};

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
shared_ptr<T, multithread>::shared_ptr(shared_ptr&& rhs)
: m_ptr(rhs.m_ptr)
{
	rhs.m_ptr = nullptr;
};

template<typename T, bool multithread>
template<class U>
CYCLIC_RC_FORCE_INLINE
shared_ptr<T, multithread>::shared_ptr(const shared_ptr<U, multithread>& rhs)
: m_ptr(rhs.m_ptr)
{
	init();
};

template<typename T, bool multithread>
template<class U>
CYCLIC_RC_FORCE_INLINE
shared_ptr<T, multithread>::shared_ptr(shared_ptr<U, multithread>&& rhs)
: m_ptr(rhs.m_ptr)
{
	rhs.m_ptr = nullptr;
};

template<typename T, bool multithread>
inline shared_ptr<T, multithread>& 
shared_ptr<T, multithread>::operator=(const shared_ptr& rhs)
{
    using config    = typename details::make_config<multithread>::type;
    using obj_count = details::obj_count<config>;

    obj_count::update(m_ptr, rhs.m_ptr);

	return *this;
}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
shared_ptr<T, multithread>& 
    shared_ptr<T, multithread>::operator=(shared_ptr&& rhs)
{
    shared_ptr tmp(std::move(rhs));
    this->swap(tmp);
	return *this;
}

template<typename T, bool multithread>
template<class U>
inline
shared_ptr<T, multithread>& 
    shared_ptr<T, multithread>::operator=(const shared_ptr<U, multithread> & rhs)
{
    *this = shared_ptr(rhs);
	return *this;
}

template<typename T, bool multithread>
template<class U>
CYCLIC_RC_FORCE_INLINE
shared_ptr<T, multithread>& 
    shared_ptr<T, multithread>::operator=(shared_ptr<U, multithread>&& rhs)
{
    *this = shared_ptr(std::move(rhs));
	return *this;
}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
void shared_ptr<T, multithread>::reset()
{
    this->~shared_ptr();
    m_ptr = nullptr;
}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
void shared_ptr<T, multithread>::reset(pointer_type p)
{
    if (p)
        p->get_counter().increase_refcount();

    this->~shared_ptr();
    m_ptr = p;
}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
void shared_ptr<T, multithread>::swap(shared_ptr& other)
{
    std::swap(this->m_ptr, other.m_ptr);
};

template<typename T, bool multithread>
inline
shared_ptr<T, multithread>::~shared_ptr()
{
    destroy(m_ptr);
}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
void shared_ptr<T, multithread>::destroy(slot* p)
{
    if (p)
        p->get_counter().decrease_refcount(p);
};

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
typename shared_ptr<T, multithread>::pointer_type
    shared_ptr<T, multithread>::operator->() const
{
    return m_ptr;
}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
typename shared_ptr<T, multithread>::pointer_type
    shared_ptr<T, multithread>::get() const
{
    return m_ptr;
}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
typename shared_ptr<T, multithread>::reference_type
    shared_ptr<T, multithread>::operator*() const
{
	assert((get() != NULL) && "dereffering null pointer");

	return *get();
};

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
shared_ptr<T, multithread>::operator bool() const
{
    return m_ptr ? true : false;
};

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
bool shared_ptr<T, multithread>::operator!() const
{
    return !m_ptr;
};

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
size_t shared_ptr<T, multithread>::use_count() const
{
    if (!m_ptr)
        return 0;
    else
        return m_ptr->get_counter().get_count();
}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
bool shared_ptr<T, multithread>::unique() const
{
    return this->use_count() == 1;
}

template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
void shared_ptr<T, multithread>::visit_children(int type)
{
	if(!m_ptr || m_ptr->get_counter().is_acyclic() )
		return;

    m_ptr->get_counter().do_visit_children(m_ptr, type);
};

template<typename T, bool multithread>
inline
void shared_ptr<T, multithread>::collect(bool val)
{
    using config            = typename details::make_config<multithread>::type;
    using obj_count         = details::obj_count<config>;
    return obj_count::collect(val);
};

};
