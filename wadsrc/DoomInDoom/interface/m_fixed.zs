/**
 * DoomInDoom - Doom compiled to ZScript
 * Copyright (C) 2024 spazzylemons
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

extend class DoomInDoom {
    // Based on JagDoom code, as we don't have 64-bit multiply in ZScript.
    uint func_FixedMul(uint a, uint b) {
        int sign = a ^ b;
        if (int(a) < 0)
            a = -a;

        if (int(b) < 0)
            b = -b;

        uint xl = a & 0xffff;
        uint xh = a >> 16;
        uint yl = b & 0xffff;
        uint yh = b >> 16;

        uint lo = xl * yl;
        uint mid = xh * yl + xl * yh;
        uint hi = xh * yh;

        uint last = lo;
        lo += mid << 16;
        hi += mid >> 16;

        if (lo < last) ++hi;

        if (sign < 0) {
            hi = -hi;
            if (lo) --hi;
            lo = -lo;
        }

        return (hi << 16) | (lo >> 16);
    }

    // Based on JagDoom code, as we don't have 64-bit divide in ZScript.
    uint func_FixedDiv(uint a, uint b) {
        int sign = a ^ b;
        uint aa = abs(int(a));
        uint bb = abs(int(b));

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
