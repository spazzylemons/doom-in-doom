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
    private Array<uint8> savegames[6];

    private uint saveSlot;
    private uint saveReadIndex;

    private Array<uint8> tempSave;

    void func_G_SaveLoad(uint slot) {
        saveSlot = slot;
        saveReadIndex = 0;
    }

    void func_G_SaveStart(uint slot) {
        saveSlot = slot;
        tempSave.Clear();
    }

    uint func_G_SaveRead(uint dest, uint len) {
        if (saveReadIndex + len > uint(savegames[saveSlot].Size())) {
            return 0;
        }

        while (len--) {
            Store8(dest++, savegames[saveSlot][saveReadIndex++]);
        }
        return 1;
    }

    void func_G_WriteSaveByte(uint b) {
        tempSave.Push(b);
    }

    uint func_G_SaveSize() {
        return tempSave.Size();
    }

    uint func_G_LoadSize() {
        return saveReadIndex;
    }

    void func_G_SaveCommit() {
        savegames[saveSlot] = tempSave;
        tempSave.Clear();
    }
}
