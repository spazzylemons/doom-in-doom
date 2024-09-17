const MEMORY_SIZE = 0x1800000;
const MIN_VALID_MEMORY = 1024;

class DoomInDoom : Actor {
    String linebuffer;

    Map<int, sound> sounds;

    Array<event_t> events;

    uint ticCount;

    uint stack;
    uint8 memory[MEMORY_SIZE];

    void Load() {
        LoadFuncPtrs();
    }

    uint Load1(uint addr) {
        return !!memory[addr];
    }

    uint Load8(uint addr) {
        return memory[addr];
    }

    uint Load16(uint addr) {
        uint a = memory[addr++];
        uint b = memory[addr];
        return a | (b << 8);
    }

    uint Load32(uint addr) {
        uint a = memory[addr++];
        uint b = memory[addr++];
        uint c = memory[addr++];
        uint d = memory[addr];
        return a | (b << 8) | (c << 16) | (d << 24);
    }

    void Store1(uint addr, uint value) {
        memory[addr] = value;
    }

    void Store8(uint addr, uint value) {
        memory[addr] = value;
    }

    void Store16(uint addr, uint value) {
        memory[addr++] = value;
        memory[addr] = value >> 8;
    }

    void Store32(uint addr, uint value) {
        memory[addr++] = value;
        memory[addr++] = value >> 8;
        memory[addr++] = value >> 16;
        memory[addr] = value >> 24;
    }

    uint Alloca(uint size, uint align) {
        stack -= size;
        stack &= ~(align - 1);

        return stack;
    }

    void Unreachable() {
        ThrowAbortException("Reached unreachable code");
    }

    void func_I_RV_ReadLump(uint lump, uint dest) {
        let data = Wads.ReadLump(lumps[lump]);
        for (uint i = 0; i < data.Length(); i++) {
            memory[dest++] = data.ByteAt(i);
        }
    }

    uint func_I_RV_SaveRead(uint data, uint size) {
        // TODO
        return 0;
    }

    void func_I_RV_SaveWrite(uint data, uint size) {
        // TODO
    }

    uint func_I_RV_SaveSize() {
        // TODO
        return 0;
    }

    uint func_I_RV_SaveLoad(uint id) {
        // TODO
        return 0;
    }

    void func_I_RV_SaveStart(uint id) {
        // TODO
    }

    void func_I_RV_SaveCommit() {
        // TODO
    }

    void func_I_RV_SaveClose() {
        // TODO
    }

    String GetString(uint addr) {
        String result;
        for (;;) {
            let c = Load8(addr++);
            if (!c) break;

            result.AppendCharacter(c);
        }
        return result;
    }

    void func_I_RV_Quit() {
        ThrowAbortException("Quit");
    }

    void func__putchar(uint c) {
        if (c == 10) {
            Console.Printf("%s", linebuffer);
            linebuffer = "";
        } else if (c >= 0x20 && c <= 0x7e) {
            linebuffer = String.Format("%s%c", linebuffer, c);
        }
    }

    uint func_I_GetTime() {
        return ticCount;
    }

    void Reset() {
        let rom = Wads.ReadLump(Wads.CheckNumForFullName("DoomInDoom/data.bin"));
        uint i;
        for (i = 0; i < MEMORY_SIZE; i++) {
            memory[i] = 0;
        }
        for (i = 0; i < rom.Length(); i++) {
            Store8(i + MIN_VALID_MEMORY, rom.ByteAt(i));
        }

        stack = MEMORY_SIZE;

        // Start it up!
        func_D_DoomMain();
    }

    void AddEvent(event_t e) {
        events.Push(e);
    }

    void Run() {
        func_D_RunFrame();
        ticCount++;
    }

    default {
        Height 1;
        Radius 1;

        +NOCLIP;
    }

    states {
        Spawn:
            TNT1 A 0;
            // Load the function pointers.
            TNT1 A 0 Load;
            TNT1 A 0 Reset;
            TNT1 A 1 Run;
            wait;
	}
}
