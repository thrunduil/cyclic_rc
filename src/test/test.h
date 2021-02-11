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

#include "obj.h"

#include <map>
#include <vector>

namespace cyclic_rc { namespace testing
{

template <bool multithread>
class test
{
    private:
        enum class operation_type
        {
            create_new,             destroy_existing,
            assign_left_new,        assign_right_new,
            assign_left_existing,   assign_right_existing,
            destroy_left,           destroy_right,
            change,                 store_global,
            load_global,            delete_global,
            last
        };

        enum class insert_type
        {
            global, buffer, buffer_child
        };

        //static const unsigned N   = 100;
        //static const unsigned N   = 500000;
        static const unsigned N     = 5000000;       

    private:
        using obj_ptr       = obj_ptr<multithread>;
        using obj           = obj<multithread>;
        using obj_vector    = std::vector<obj_ptr>;
        using mutex_type    = typename get_mutex<multithread>::type;

    public:
        void            make(int n_operations);
        static void     clear_global();

    private:
        operation_type  rand_op();
        int             rand_pos();
        static int      rand_global_pos();
        bool            make_clear_all();

        void            op_create_new();
        void            op_destroy_existing();
        void            op_assign_left_new();
        void            op_assign_right_new();
        void            op_assign_left_existing();
        void            op_assign_right_existing();
        void            op_destroy_left();
        void            op_destroy_right();
        void            op_change();
        void            op_store_global();
        void            op_load_global();
        void            op_delete_global();
        void            op_clear_all();

        bool            do_global() const;

        static void     store_global(const obj_ptr& obj);
        static obj_ptr  load_global();

    private:
        obj_vector          m_obj_vector;
        static obj_vector   m_obj_vector_global;
        static mutex_type*  m_global_mutex;
};

};}