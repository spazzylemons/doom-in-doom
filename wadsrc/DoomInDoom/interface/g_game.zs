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
