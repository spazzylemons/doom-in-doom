//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Fixed point implementation.
//



#include "stdlib.h"

#include "doomtype.h"
#include "i_system.h"

#include "m_fixed.h"




// Fixme. __USE_C_FIXED__ or something.

fixed_t
FixedMul
( fixed_t	a,
  fixed_t	b )
{
	fixed_t result;
	asm volatile (
		".insn r 0x73, 0x4, 0x43, %[out], %[a], %[b]"
		: [out]"=r"(result)
		: [a]"r"(a), [b]"r"(b)
	);
	return result;
}



//
// FixedDiv, C version.
//

fixed_t FixedDiv(fixed_t a, fixed_t b) {
	fixed_t result;
	asm volatile (
		".insn r 0x73, 0x4, 0x41, %[out], %[a], %[b]"
		: [out]"=r"(result)
		: [a]"r"(a), [b]"r"(b)
	);
	return result;
}

