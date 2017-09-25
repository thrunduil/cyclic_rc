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

#include "ref_count.h"
#include <cassert>

namespace cyclic_rc { namespace details
{

inline rc_count::ref_info::ref_info(bool is_acyclic)
    : count(0), buffered(0), age((int)age_type::old)
    , color(is_acyclic ? (size_t)color::green : (size_t)color::black)    
{};

inline  rc_count::rc_count(bool is_acyclic)
    :m_ref_info(is_acyclic)
{};

inline size_t rc_count::get_count() const
{
    return m_ref_info.count;
}

inline bool rc_count::is_count_zero() const
{
    return get_count() == 0;
};

inline bool rc_count::is_acyclic() const
{
    return m_ref_info.color == (int)color::green;
};

inline bool rc_count::is_purple() const
{
    return m_ref_info.color == (int)color::purple;
};

inline bool rc_count::is_black() const
{
    return m_ref_info.color == (int)color::black;
};

inline bool rc_count::is_gray() const
{
    return m_ref_info.color == (int)color::gray;
};

inline bool rc_count::is_white() const
{
    return m_ref_info.color == (int)color::white;
};

inline bool rc_count::is_buffered() const
{
    return m_ref_info.buffered == 1;
};

inline bool rc_count::is_young() const
{
    return m_ref_info.age == (int)age_type::young;
};

inline bool rc_count::is_medium() const
{
    return m_ref_info.age == (int)age_type::medium;
};

inline bool rc_count::is_old() const
{
    return m_ref_info.age == (int)age_type::old;
};

inline void rc_count::increase_count()
{
    ++m_ref_info.count;
};

inline size_t rc_count::decrease_count()
{
    assert(m_ref_info.count > 0);
    return --m_ref_info.count;
};

inline void rc_count::mark_black()
{
    m_ref_info.color = (int)color::black;
};

inline void rc_count::mark_gray()
{
    m_ref_info.color = (int)color::gray;
}

inline void rc_count::mark_white()
{
    m_ref_info.color = (int)color::white;
};

inline void rc_count::mark_purple()
{
    m_ref_info.color = (int)color::purple;
};

inline void rc_count::mark_buffered()
{
    m_ref_info.buffered = 1;
};;

inline void rc_count::mark_nonbuffered()
{
    m_ref_info.buffered = 0;
};

inline void rc_count::mark_age(age_type age)
{
    m_ref_info.age = (int)age;
};

}}