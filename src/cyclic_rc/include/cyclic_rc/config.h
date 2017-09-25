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

// when defined, then memory leaks debugging is enabled
#ifdef _DEBUG
    #define CYCLIC_RC_TEST 1
#else
    #define CYCLIC_RC_TEST 0
#endif

// dll export/import macro
#ifdef CYCLIC_RC_EXPORTS 
    #define CYCLIC_RC_EXPORT _declspec(dllexport)
#else 
    #define CYCLIC_RC_EXPORT _declspec(dllimport)
#endif

#define CYCLIC_RC_FORCE_INLINE __forceinline