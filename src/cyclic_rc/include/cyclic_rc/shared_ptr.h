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

#include "cyclic_rc/details/obj_count.h"

namespace cyclic_rc { namespace details
{

template<class config>
class obj_count;

template<class config>
class collector;

}}

namespace cyclic_rc
{

template<class T>
struct is_multithreaded
{
    static const bool value = T::is_multithreaded;
};

// base class of objects, that can be managed by shared_ptr class
// if multithread = true, then thread-safe version of the collector is 
// used
template<bool multithread>
class cyclic_rc_base
{
    private:      
        using config_type   = typename details::make_config<multithread>::type;
        using counter_type  = details::obj_count<config_type>;

    public:
        static const bool is_multithreaded  = multithread;

    private:
        counter_type        m_counter;

    public:
        // initialize reference counter to zero; if is_acyclic = true, then it is
        // assumed, that this object cannot be part of garbage cycle
        cyclic_rc_base(bool is_acyclic = false);

        // destructor
        virtual ~cyclic_rc_base(){};

        // function implementing accessibility test; impementation must call
        // shared_ptr::visit_children(t) function on all directly accessible
        // objects (even if stored in other, not managed object) where t = type
        // is argument of this function. It is safe to call this function many
        // times on the same object. If this function is not called on some
        // accessible object, then memory leaks are possible
        virtual void        visit_children(int type) = 0;

    private:
        const counter_type& get_counter() const { return m_counter; };
        counter_type&       get_counter()       { return m_counter; };        

        template<class config>
        friend class details::collector;

        template<class T, bool mt>
        friend class shared_ptr;

        template<class config>
        friend class details::obj_count;
};

// reference counting handling circular memory references
//
// cyclic_rc::shared_ptr class offers similar functionality to std::shared_ptr,
// but is able to reclaim memory in case of reference cycles. This class retains 
// shared ownership of an object through a pointer. Several shared_ptr objects may 
// own the same object. The object is destroyed and its memory deallocated when:
//  1. reference count drops to zero , or
//  2. reference count is nonzero, but during cycle collection phase it is 
//      discovered, that this object belongs to mutually referencing group of 
//      objects, not referenced by any object outside this group.
// 
// During collection phase a set of possibly no longer accessible objects is 
// created. Next all objects referenced by objects from this set are visited. The 
// user must provide a function, that perform this traversal.
//
// Managed objects of type T must derive from class cyclic_rc_base<multithreaded>.
// 
// cyclic_rc::shared_ptr can work in multithreaded environment, however garbage
// collection is blocking. When multithreaded = true, then multi-threaded version
// is used. 
//
// Collection algorithm is based on:
//  "A Pure Reference Counting Garbage Collector",  DAVID F. BACON, CLEMENT R. 
//  ATTANASIO, V.T. RAJAN, STEPHEN E. SMITH
template<typename T, bool multithread = is_multithreaded<T>::value>
class shared_ptr
{
    private:    
        // type of the managed object
        using value_type    = T;

        // pointer type of managed object
        using pointer_type  = T*;

        // reference type of managed object
        using reference_type = T&;
        
    private:
        using slot          = T;

    public:
        // create empty object
        shared_ptr();

        // create empty object
        shared_ptr(nullptr_t);

        // initialize with pointer p; reference counter increased by one
        explicit shared_ptr(pointer_type p);

        // initialize with pointer p of other type
        template<class U>
        explicit shared_ptr(U* p);

        // copy constructor; if other is not empty, then reference counter
        // is increased; otherwise empty object is created
        shared_ptr(const shared_ptr& other);

        // move constructor; pinter stored in other is move to this object;
        // other becomes na empty object
        shared_ptr(shared_ptr&& other);

        // copy constructor from shared_ptr of other type
        template<class Y>
        shared_ptr(shared_ptr<Y, multithread> const& r);

        // move costructor from shared_ptr of other type
        template<class Y>
        shared_ptr(shared_ptr<Y, multithread> && r);

        // destructor; destructor of stored pointer is called, if reference
        // counter drops to zero (possibly after destroying all objects accessible
        // from stored pointed); destructor of stored pointer will be called
        // during next collection phase
        ~shared_ptr();        
        
        // copy assignments; reference counting of other is increased by one
        // and destructor of this objet is called
        shared_ptr&         operator=(const shared_ptr& other);

        // move assignments; ownership from other is transfered to this object
        // without altering reference counter; destructor of this object is called;
        // other becomes an empty object
        shared_ptr&         operator=(shared_ptr&& other);

        // copy assignment from shared_ptr of other type
        template<class Y>
        shared_ptr&         operator=(shared_ptr<Y, multithread> const& r);

        // move assignment from shared_ptr of other type
        template<class Y>
        shared_ptr&         operator=(shared_ptr<Y, multithread> && r);

        // destructor of this object is called, and stored pointer is set to
        // nullptr; equivalent to operator=(shared_ptr())
        void                reset();

        // destructor of this object is called, stored pointer is set to
        // p and reference counter is increased; equivalent to operator=(shared_ptr(p))
        void                reset(pointer_type p);

        // exchange contents of other object and this object without altering
        // reference counters
        void                swap(shared_ptr& other);

        // get stored pointer
        pointer_type        get() const;

        // return stored pointer in order to access one of its members; this object
        // cannot be empty
        pointer_type        operator->() const;

        // return a reference to stored object; equivalent to *get(); this object
        // cannot be empty
        reference_type      operator*() const;

        // cast operator to boolean value
        explicit            operator bool() const;

        // boolean negation operator
        bool                operator!() const;

        // return reference count (return 0 for empty objects)
        size_t              use_count() const;

        // return use_count() == 1
        bool                unique() const;

        // call collector function on stored pointer; stored pointed should
        // call visit_children function on all directly accessible objects
        void                visit_children(int type);

        // force collection of no longer accessible objects; if all = true, 
        // then destructors of all inaccessible objects will be called; 
        // otherwise some destructors may be delayed
        static void			collect(bool all);

    private:
        void                init();
        void                destroy(slot* p);

    private:
        slot*               m_ptr;

    private:
        // copy constructor from shared_ptr of other kind is disabled
        template<class Y, bool multi2>
        shared_ptr(shared_ptr<Y, multi2> const& r) = delete;

        // move costructor from shared_ptr of other kind is disabed
        template<class Y, bool multi2>
        shared_ptr(shared_ptr<Y, multi2> && r) = delete;

        // copy assignment from shared_ptr of other kind is disabled
        template<class Y, bool multi2>
        shared_ptr&         operator=(shared_ptr<Y, multi2> const& r) = delete;

        // move assignment from shared_ptr of other kind is disabled
        template<class Y, bool multi2>
        shared_ptr&         operator=(shared_ptr<Y, multi2> && r) = delete;

        template<class T, bool multithread>
        friend class shared_ptr;
};

// exchange contents of other object and this object without altering
// reference counters
template<typename T, bool multithread>
CYCLIC_RC_FORCE_INLINE
void swap(shared_ptr<T, multithread>& a, shared_ptr<T, multithread>& b)
{
    a.swap(b);
}

};

#include "cyclic_rc/details/shared_ptr.inl"
