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
//	Fixed point arithemtics, implementation.
//

//
// Fixed point, 32bit as 16.16.
//
const FRACBITS = 16;
const FRACUNIT = (1 << FRACBITS);

struct Fixed {
    // Based on JagDoom code, as we don't have 64-bit multiply in ZScript.
    static int Mul(int a, int b) {
        int sign = a ^ b;
        if (a < 0)
            a = -a;

        if (b < 0)
            b = -b;

        uint a1 = a & 0xffff;
        uint a2 = uint(a) >> 16;
        uint b1 = b & 0xffff;
        uint b2 = uint(b) >> 16;
        uint c = (a1 * b1) >> 16;
        c += a2 * b1;
        c += b2 * a1;
        c += (b2 * a2) << 16;

        if (sign < 0)
            c = -c;

        return c;
    }

    // Based on JagDoom code, as we don't have 64-bit divide in ZScript.
    static int Div(int a, int b) {
        int sign = a ^ b;
        uint aa, bb;

        if (a < 0) {
            aa = -a;
        } else {
            aa = a;
        }

        if (b < 0) {
            bb = -b;
        } else {
            bb = b;
        }

        if ((aa >> 14) >= bb) {
            return sign < 0 ? 0x80000000 : 0x7fffffff;
        }

        uint bit = 0x10000;
        while (aa > bb) {
            bb <<= 1;
            bit <<= 1;
        }

        uint c = 0;

        do {
            if (aa >= bb) {
                aa -= bb;
                c |= bit;
            }
            aa <<= 1;
            bit >>= 1;
        } while (bit && aa);

        if (sign < 0)
            c = -c;

        return c;
    }
}
