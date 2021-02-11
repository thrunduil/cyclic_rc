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

#pragma warning(push)
#pragma warning(disable: 4251) // needs to have dll-interface to be used by clients

namespace cyclic_rc
{

template<bool multithread>
class cyclic_rc_base;

};

namespace cyclic_rc { namespace details
{

template<class config>
class obj_count;

template<class config, bool multithreaded>
struct collector_is_in_free{};

template<class config>
struct collector_is_in_free<config, true>
{
    thread_local
    static bool value;
};

template<class config>
struct collector_is_in_free<config, false>
{
    static bool value;
};

template<class config>
class CYCLIC_RC_EXPORT collector
{
    private:
        static const bool multithreaded = config::is_multithreaded;

	private:
        using slot_base                 = cyclic_rc_base<multithreaded>;
        using obj_count                 = obj_count<config>;
        using root_vector               = std::vector<slot_base*>;

        static const int n_medium       = 5;
        static const int threshold      = 2000;

	private:
		root_vector*        m_objects_old;
        root_vector*        m_objects_medium[n_medium];
        root_vector*        m_objects_young;
        root_vector         m_objects_to_free;

		bool				collecting;
		double				allocated_memory;

		void				mark();
		void				scan();
		void				collect_roots();
        bool                process_buffers();
        void                process_free_objects();
		
		void				add_young_impl(slot_base* s);
		
		void				collect_impl(bool collect_all);	
		void				start_collector_if_required();        

		collector();
		~collector();

        friend struct collector_initializer;

	public:
		static void			add_young(slot_base* s);		
        static void         free_object(slot_base* s);
        static bool         is_freeing();
        static void			make_collect(bool all);

    private:
        static collector*   get();
};

struct CYCLIC_RC_EXPORT collector_initializer
{
    collector_initializer();
    ~collector_initializer();
};

static collector_initializer collector_init;

}}

#pragma warning(pop)