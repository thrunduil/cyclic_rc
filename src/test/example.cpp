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

#include "cyclic_rc/shared_ptr.h"
#include <iostream>

using namespace cyclic_rc;

// forward declarations
struct tree;
using tree_ptr  = shared_ptr<tree, false>;

// class managed by shared_ptr in single-thread mode
struct tree : cyclic_rc_base<false>
{
    tree_ptr    left;
    tree_ptr    right;

    ~tree()
    {
        std::cout << "tree destroyed" << "\n";
    };

    // implementation of traversal function
    void visit_children(int t) override
    {
        left.visit_children(t);
        right.visit_children(t);
    };
};

void example()
{
    // create reference cycle
    tree_ptr root = tree_ptr(new tree());

    root->left  = root;
    root->right = root;
    
    std::cout << "object no longer accessible" << "\n";
    root = tree_ptr();

    std::cout << "collector called" << "\n";
    
    // force garbage collection
    tree_ptr::collect(true);
};
