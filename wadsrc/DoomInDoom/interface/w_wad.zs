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
    private Array<int> lumps;

    uint func_W_Load(void) {
        uint numlumps = Wads.GetNumLumps();

        // Special handling for namespaced lumps to ensure ordering is correct
        // when multiple namespaced sections appear.
        // Have fun playing Knee-Deep in Knee-Deep in ZDoom in Doom in ZDoom.

        Map<String, int> spritesSeen;
        Array<int> spriteLumps;
        Map<String, int> flatsSeen;
        Array<int> flatLumps;

        for (uint i = 0; i < numlumps; i++) {
            let namespace = Wads.GetLumpNamespace(i);
            let lumpName = Wads.GetLumpName(i);
            // Check that it's a vanilla lump.
            if (namespace < 0 || Wads.GetLumpFullName(i) != lumpName)
                continue;
            // Ignore the namespace separators - we'll insert them ourselves.
            if (lumpName == "S_START" || lumpName == "S_END" || lumpName == "F_START" || lumpName == "F_END")
                continue;
            // Check namespace.
            switch (namespace) {
                case 0:
                    lumps.Push(i);
                    break;
                case 1:
                    if (!spritesSeen.CheckKey(lumpName)) {
                        spritesSeen.Insert(lumpName, spriteLumps.Size());
                        spriteLumps.Push(i);
                    } else {
                        spriteLumps[spritesSeen.Get(lumpName)] = i;
                    }
                    break;
                case 2:
                    if (!flatsSeen.CheckKey(lumpName)) {
                        flatsSeen.Insert(lumpName, flatLumps.Size());
                        flatLumps.Push(i);
                    } else {
                        flatLumps[flatsSeen.Get(lumpName)] = i;
                    }
                    break;
            }
        }

		// Build namespaces.
        lumps.Push(Wads.CheckNumForName("S_START", Wads.GlobalNamespace));
        lumps.Append(spriteLumps);
        lumps.Push(Wads.CheckNumForName("S_END", Wads.GlobalNamespace));

        lumps.Push(Wads.CheckNumForName("F_START", Wads.GlobalNamespace));
        lumps.Append(flatLumps);
        lumps.Push(Wads.CheckNumForName("F_END", Wads.GlobalNamespace));

        return lumps.Size();
    }

    void func_W_ReadLumps(uint addr) {
        let numlumps = lumps.Size();
        for (let i = 0; i < numlumps; i++) {
            let name = Wads.GetLumpName(lumps[i]);
            uint j;
            for (j = 0; j < name.Length() && j < 8; j++) {
                Store8(addr++, name.ByteAt(j));
            }
            for (; j < 8; j++) {
                Store8(addr++, 0);
            }
            Store32(addr, Wads.ReadLump(lumps[i]).Length());
            addr += 12;
        }
    }

    void func_W_ReadLump(uint lump, uint dest) {
        let data = Wads.ReadLump(lumps[lump]);
        for (uint i = 0; i < data.Length(); i++) {
            Store8(dest++, data.ByteAt(i));
        }
    }
}
