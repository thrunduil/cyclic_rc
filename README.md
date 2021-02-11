# cyclic_rc

Reference counting handling circular memory references.

cyclic_rc :: shared_ptr class offers similar functionality to std :: shared_ptr,
but is able to reclaim memory in case of reference cycles. This class retains 
shared ownership of an object through a pointer. Several shared_ptr objects may 
own the same object. The object is destroyed and its memory deallocated when:
 1. reference count drops to zero , or
 2. reference count is nonzero, but during cycle collection phase it is 
     discovered, that this object belongs to mutually referencing group of 
     objects, not referenced by any object outside this group.

During collection phase a set of possibly no longer accessible objects is 
created. Next all objects referenced by objects from this set are visited. The 
user must provide a function, that perform this traversal.

cyclic_rc :: shared_ptr can work in multithreaded environment, however garbage
collection is blocking.

References:

 [1] "A Pure Reference Counting Garbage Collector", 2001,
    Bacon, D. F., Attanasio, C. R., Rajan, V. T., Smith, S. E., & LEE, H.

## Code example

Simple bitset operations:
```cpp
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

Output:
    object no longer accessible
    collector called
    tree destroyed
```            

## Licence

This library is published under GPL licence.