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
    private Canvas canvas;
    private Color palette[256];

    void func_I_GetCanvas() {
        canvas = TexMan.GetCanvas("DOOMSCRN");
    }

    void func_I_SetPalette(uint addr) {
        for (uint i = 0; i < 256; i++) {
            let r = memory[addr++];
            let g = memory[addr++];
            let b = memory[addr++];
            palette[i] = Color(r, g, b);
        }
    }

    void func_I_GetEvent() {
        let s = stack;
        let ev = Alloca(16, 4);

        while (events.Size() != 0) {
            let event = events[0];
            events.Delete(0);

            Store32(ev, event.type);
            Store32(ev + 4, event.data1);
            Store32(ev + 8, event.data2);
            Store32(ev + 12, event.data3);
            func_D_PostEvent(ev);
        }

        stack = s;
    }

    void func_I_DrawScreen(uint addr) {
        for (uint y = 0; y < 200; y++) {
            for (uint x = 0; x < 320; x++) {
                canvas.Clear(x, y, x + 1, y + 1, palette[memory[addr++]]);
            }
        }
    }
}
