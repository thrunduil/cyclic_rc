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

#include "collector.h"
#include "cyclic_rc/details/ref_count.inl"
#include "cyclic_rc/details/obj_count.h"

namespace cyclic_rc { namespace details
{

template<class config>
inline 
void collector<config>::add_young(slot_base* s)
{
	get()->add_young_impl(s);
};

template<class config>
inline 
void collector<config>::add_young_impl(slot_base* s)
{
    m_objects_young->push_back(s);
	start_collector_if_required();
};

template<class config>
inline 
void collector<config>::start_collector_if_required()
{
	if (!collecting && m_objects_young->size() >= threshold)
		collect_impl(false);
};

template<class config>
inline
bool collector<config>::is_freeing()
{
    using is_free_type  = collector_is_in_free<config, multithreaded>;
    return is_free_type::value;
};

template<class config>
inline
void collector<config>::make_collect(bool all)
{
	collector<config>::get()->collect_impl(all);
};

template<class config>
inline
void collector<config>::free_object(slot_base* s)
{
    collector<config>::get()->m_objects_to_free.push_back(s);
};

using collector_in = collector<config_nothread>;
using collector_it = collector<config_thread>;

extern collector_in * g_collector_in;
extern collector_it * g_collector_it;

inline collector<config_nothread>* 
collector<config_nothread>::get()
{
    return g_collector_in;
};

inline collector<config_thread>* 
collector<config_thread>::get()
{
    return g_collector_it;
};

}}