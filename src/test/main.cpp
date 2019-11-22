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

#include "test.h"
#include "obj.h"
#include "timer.h"
#include <iostream>

#include <thread>
#include <functional>

using namespace cyclic_rc;
using namespace cyclic_rc :: testing;

#pragma warning(push)
#pragma warning(disable: 4127) // conditional expression is constant

void example();

template<bool multithread>
void test_func()
{
    #ifdef _DEBUG
        int N1  = 1;
        int N2  = 500000;
        //int N2  = 20;
    #else
        int N1  = 1;
        int N2  = 5000000;
    #endif

    timer  t;
    for (int i = 0; i < N1; ++i)
    {
        t.tic();
        {
            test<multithread> t2;
            t2.make(N2);
        };
        t.tocdisp();
    };
};

template<bool multithread>
void main_test()
{
    using obj       = obj<multithread>;
    using obj_ptr   = obj_ptr<multithread>;

    for (int i = 0; i < 10; ++i)
    {
        {
            std::thread t1 = std::thread(std::function<void()>(&test_func<multithread>));
            std::thread t2; 
            std::thread t3; 
            std::thread t4; 
        
            if (multithread == true)
            {
                t2 = std::thread(std::function<void()>(&test_func<multithread>));
                t3 = std::thread(std::function<void()>(&test_func<multithread>));
                t4 = std::thread(std::function<void()>(&test_func<multithread>));
            }

            t1.join();

            if (multithread == true)
            {
                t2.join();
                t3.join();
                t4.join();
            }

            test<multithread>::clear_global();
        };

        #if CYCLIC_RC_TEST
            size_t n_elem  = obj::n_counters();
            (void)n_elem;

            obj_ptr::collect(true);
            if (obj::n_counters() > 0)
            {
                std::cout << "memory leaks!\n" << obj::n_counters()
                            << "\n";

                obj_ptr::collect(true);
                if (obj::n_counters() > 0)
                {
                    std::cout << "memory leaks  2!\n" << obj::n_counters()
                                << "\n";
                };
            };
        #endif
    };
};

int main(int argc, char* argv[])
{    
    (void)argc;
    (void)argv;

    example();

    srand(0);

    #if CYCLIC_RC_TEST
        std::cout << "\n" << "leak detection enabled" << "\n";
        std::cout << "found leaks will be reported" << "\n";
    #else
        std::cout << "\n" << "leak detection not enabled" << "\n";
    #endif

    std::cout << "\n" << "TESTING: single-thread" << "\n";
    main_test<false>();   

    std::cout << "\n" << "TESTING: multi-thread" << "\n";
    main_test<true>();

    std::cout << "\n" << "finished" << "\n";
	return 0;
}

#pragma warning(pop)