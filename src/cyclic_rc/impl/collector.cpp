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

#include "cyclic_rc/details/collector.inl"
#include "cyclic_rc/details/obj_count.inl"
#include "cyclic_rc/shared_ptr.h"

#include <iostream>
#include <boost/pool/pool.hpp>

namespace cyclic_rc { namespace details
{

//------------------------------------------------------------
//                      collector_initializer
//------------------------------------------------------------
collector_in * g_collector_in = nullptr;
collector_it * g_collector_it = nullptr;

// nifty counter
static int g_counter = 0;

bool collector_is_in_free<config_nothread, false>::value      = false;

thread_local
bool collector_is_in_free<config_thread, true>::value         = false;

using obj_count_in  = obj_count<config_nothread>;
using obj_count_it  = obj_count<config_thread>;

//must be alive during global objects destruction
obj_count_in::mutex_type* obj_count_in::m_mutex = nullptr;
obj_count_it::mutex_type* obj_count_it::m_mutex = nullptr;

collector_initializer::collector_initializer()
{
    if (g_counter == 0)
    {
        obj_count_in::m_mutex = new obj_count_in::mutex_type();
        obj_count_it::m_mutex = new obj_count_it::mutex_type();

        g_collector_in = new collector_in();
        g_collector_it = new collector_it();
    };

    ++g_counter;
};

collector_initializer::~collector_initializer()
{
    --g_counter;

    if (g_counter == 0)   
    {
        delete g_collector_in;
        delete g_collector_it;

        g_collector_in = nullptr;
        g_collector_it = nullptr;

        delete obj_count_in::m_mutex;
        delete obj_count_it::m_mutex;

        obj_count_in::m_mutex = nullptr;
        obj_count_it::m_mutex = nullptr;
    };
}

//------------------------------------------------------------
//                      collector
//------------------------------------------------------------

template<class config>
void collector<config>::mark()
{
    size_t pos      = 0;
    size_t size     = m_objects_old->size();

	while(pos < size)
	{
		auto ro     = (*m_objects_old)[pos];

        if (ro->get_counter().is_old() == false)
        {
        }
        else if (ro->get_counter().is_purple() && ro->get_counter().get_cout_impl() > 0)
		{
            ro->get_counter().mark_gray(ro);
			++pos;
            continue;
		}
		else
		{
            ro->get_counter().mark_nonbuffered();			

			if(ro->get_counter().is_black() && ro->get_counter().is_count_zero())
                free_object(ro);
		};		

        (*m_objects_old)[pos]   = m_objects_old->back();

        m_objects_old->pop_back();
        --size;
	};
};

template<class config>
void collector<config>::scan()
{
	for(size_t i = 0; i < m_objects_old->size(); ++i)
        (*m_objects_old)[i]->get_counter().scan((*m_objects_old)[i]);
};

template<class config>
void collector<config>::collect_roots()
{
	for (size_t i = 0; i < m_objects_old->size(); ++i)
	{
	    (*m_objects_old)[i]->get_counter().mark_nonbuffered();
        (*m_objects_old)[i]->get_counter().collect_white((*m_objects_old)[i]);
	};	

    m_objects_old->clear();
};

template<class config>
bool collector<config>::process_buffers()
{
    size_t pos;
    size_t size;

    for (int i = 0; i < n_medium; ++i)
    {
        root_vector& vec        = *m_objects_medium[i];
        details::age_type age   = (i == n_medium - 1) ? details::age_type::old 
                                                      : details::age_type::medium; 

        pos     = 0;
        size    = vec.size();

        while (pos < size)
        {
            obj_count& tmp = vec[pos]->get_counter();

            if (tmp.is_young() == true || tmp.is_buffered() == false)
            {
            }
            else if (tmp.is_black() == true )                
            {
                tmp.mark_nonbuffered();

                if (tmp.get_cout_impl() == 0)
                {
                    //object is unrecheable, destroy                    
                    this->free_object(vec[pos]);
                }
                else
                {
                    //object was accessed, not a garbage
                };
            }
            else
            {
                tmp.mark_age(age);
                ++pos;
                continue;
            };

            vec[pos] = vec.back();
            vec.pop_back();
            --size;

        };
    };

    pos = 0;
    size = m_objects_young->size();

    while (pos < size)
    {
        obj_count& tmp = (*m_objects_young)[pos]->get_counter();

        tmp.mark_age(details::age_type::medium);
        
        if (tmp.is_buffered() == false)
        {
            //can be marked as nonbuffered during collect_white
        }
        else if (tmp.is_black() == true)
        {
            tmp.mark_nonbuffered();

            if (tmp.get_cout_impl() == 0)
            {                
                this->free_object((*m_objects_young)[pos]);
            }
            else
            {
                //remove from the list
            };
        }
        else
        {            
            ++pos;
            continue;
        };

        (*m_objects_young)[pos] = m_objects_young->back();
        m_objects_young->pop_back();
        --size;
    };

    //swap buffers
    root_vector* prev       = m_objects_young;
    
    for (int i = 0; i < n_medium; ++i)
    {
        root_vector* tmp    = m_objects_medium[i];
        m_objects_medium[i] = prev;
        prev                = tmp;
    };
    
    m_objects_young     = std::move(m_objects_old);
    m_objects_old       = std::move(prev);

    for (int i = 0; i < n_medium; ++i)
    {
        if (m_objects_medium[i]->size() > 0)
            return true;
    };

    if (m_objects_old->size() > 0)
        return true;

    return false;
};

template<class config>
void collector<config>::process_free_objects()
{
    using is_free_type  = collector_is_in_free<config, multithreaded>;

    is_free_type::value = true;
    size_t n            = m_objects_to_free.size();

    for (size_t i = 0; i < n; ++i)
    {
        slot_base* ptr = m_objects_to_free[i];
        ptr->get_counter().call_destructor(ptr);
    };
    
    m_objects_to_free.clear();

    is_free_type::value = false;
};

template<class config>
void collector<config>::collect_impl(bool collect_all)
{
	if (collecting == true)
		return;

	collecting				= true;

    int n                   = (collect_all? 2 + n_medium: 1);

    process_free_objects();

    for (int i = 0; i < n; ++i)
    {
	    mark();
	    scan();
	    collect_roots();

        process_buffers();
    };

    process_free_objects();

	collecting				= false;
};

template<class config>
collector<config>::collector()
{
	collecting          = false;
	allocated_memory    = 0;

    m_objects_old       = new root_vector();
    m_objects_young     = new root_vector();

    for (int i = 0; i < n_medium; ++i)
        m_objects_medium[i] = new root_vector();
};

template<class config>
collector<config>::~collector()
{
	collect_impl(true);
};

template collector<config_nothread>;
template collector<config_thread>;

};};