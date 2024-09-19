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

class VAList play {
    Array<uint> values;
    DoomInDoom d;

    static VAList Create(DoomInDoom d) {
        let result = new('VAList');
        result.d = d;
        return result;
    }

    VAList Add(uint arg) {
        values.Push(arg);
        return self;
    }

    void Start(uint ptr) {
        uint block = d.Alloca(values.Size() << 2, 4);
        for (int i = 0; i < values.Size(); i++) {
            d.Store32(block + (i << 2), values[i]);
        }
        d.Store32(ptr, block);
    }

    void End(uint ptr) {
        d.Store32(ptr, 0);
    }
}
