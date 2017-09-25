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

namespace cyclic_rc { namespace details
{

enum class age_type
{
    young   = 0,
    medium  = 1,
    old     = 2
};

class rc_count
{
    public:
        rc_count(bool is_acyclic);

        size_t              get_count() const;

        bool                is_count_zero() const;
        bool                is_acyclic() const;
        bool                is_purple() const;
        bool                is_black() const;
        bool                is_gray() const;
        bool                is_white() const;
        bool                is_buffered() const;
        bool                is_young() const;
        bool                is_medium() const;
        bool                is_old() const;

        void                increase_count();
        size_t              decrease_count();

        void                mark_black();
        void                mark_gray();
        void                mark_white();
        void                mark_purple();
        void                mark_buffered();
        void                mark_nonbuffered();
        void                mark_age(age_type age);

    private:
        enum class color
        {
            black   = 0,    // in use or free
            green   = 1,    // acyclic
            gray    = 2,    // possible member of cycle
            white   = 3,    // member of garbage cycle
            purple  = 4,    // possible root of cycle            
        };

        struct  ref_info
		{
			size_t count        : sizeof(size_t) * 8 - 6;
            size_t color        : 3;            
			size_t buffered     : 1;
            size_t age	        : 2;

			ref_info(bool is_acyclic);
		};
        
        ref_info            m_ref_info;
};

};};

#include "ref_count.inl"
