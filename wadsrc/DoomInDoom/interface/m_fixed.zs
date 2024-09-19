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
