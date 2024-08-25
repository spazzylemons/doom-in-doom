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
    return ((int64_t) a * (int64_t) b) >> FRACBITS;
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
	// int32_t sign = a ^ b;
	// uint32_t aa, bb;

	// if (a < 0) {
	// 	aa = -a;
	// } else {
	// 	aa = a;
	// }

	// if (b < 0) {
	// 	bb = -b;
	// } else {
	// 	bb = b;
	// }

	// if ((aa >> 14) >= bb) {
	// 	return sign < 0 ? INT_MIN : INT_MAX;
	// }

	// uint32_t bit = FRACUNIT;
	// while (aa > bb) {
	// 	bb <<= 1;
	// 	bit <<= 1;
	// }

	// uint32_t c = 0;

	// do {
	// 	if (aa >= bb) {
	// 		aa -= bb;
	// 		c |= bit;
	// 	}
	// 	aa <<= 1;
	// 	bit >>= 1;
	// } while (bit && aa);

	// if (sign < 0)
	// 	c = -c;

    // return c;
}

