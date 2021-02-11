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

#include <string>

namespace cyclic_rc { namespace testing
{

using Integer64 = __int64;
using Real      = double;

namespace details
{

struct timer_base
{
	bool            tic_started;
	Integer64       tic_int64;

	void			init();
	void			tic(void);
	Real			toc(void);

	std::string		tocstr(void);
	void			tocdisp(void);
};

};

class timer : private details::timer_base
{
	public:
		timer()		{	init();	};

		using details::timer_base::tic;
		using details::timer_base::toc;
		using details::timer_base::tocstr;
		using details::timer_base::tocdisp;
};

}}
