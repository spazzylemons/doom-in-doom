const MEMORY_SIZE = 0x1800000;
const MIN_VALID_MEMORY = 1024;

class DoomInDoom : Actor {
    String linebuffer;

    Array<int> lumps;
    Map<int, sound> sounds;

    Array<event_t> events;

    uint ticCount;

    Canvas canvas;

    Actor channels[8];

    Color palette[256];

    uint stack;
    uint8 memory[MEMORY_SIZE];

    void Load() {
        uint numlumps = Wads.GetNumLumps();

        // Special handling for namespaced lumps to ensure ordering is correct
        // when multiple namespaced sections appear.
        // Have fun playing Knee-Deep in Knee-Deep in ZDoom in Doom in ZDoom.

        Map<String, int> spritesSeen;
        Array<int> spriteLumps;
        Map<String, int> flatsSeen;
        Array<int> flatLumps;

        uint i;

        for (i = 0; i < numlumps; i++) {
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

        canvas = TexMan.GetCanvas("DOOMSCRN");

        for (i = 0; i < 8; i++) {
            channels[i] = Actor.Spawn('DoomSFX', pos);
        }

        LoadFuncPtrs();
    }

    uint Load1(uint addr) {
        if (addr < MIN_VALID_MEMORY) ThrowAbortException("NULL READ 1");
        if (memory[addr] > 1) ThrowAbortException("BAD BOOLEAN");
        return !!memory[addr];
    }

    uint Load8(uint addr) {
        if (addr < MIN_VALID_MEMORY) ThrowAbortException("NULL READ 8");
        return memory[addr];
    }

    uint Load16(uint addr) {
        if (addr < MIN_VALID_MEMORY) ThrowAbortException("NULL READ 16");
        uint a = memory[addr++];
        uint b = memory[addr];
        return a | (b << 8);
    }

    uint Load32(uint addr) {
        if (addr < MIN_VALID_MEMORY) ThrowAbortException("NULL READ 32");
        uint a = memory[addr++];
        uint b = memory[addr++];
        uint c = memory[addr++];
        uint d = memory[addr];
        return a | (b << 8) | (c << 16) | (d << 24);
    }

    void Store1(uint addr, uint value) {
        if (addr < MIN_VALID_MEMORY) ThrowAbortException("NULL WRITE 1");
        if (value > 1) ThrowAbortException("BAD WRITE 1");
        memory[addr] = value;
    }

    void Store8(uint addr, uint value) {
        if (addr < MIN_VALID_MEMORY) ThrowAbortException("NULL WRITE 8");
        if (value > 0xff) ThrowAbortException("BAD WRITE 8");
        memory[addr] = value;
    }

    void Store16(uint addr, uint value) {
        if (addr < MIN_VALID_MEMORY) ThrowAbortException("NULL WRITE 16");
        if (value > 0xffff) ThrowAbortException("BAD WRITE 16");
        memory[addr++] = value;
        memory[addr] = value >> 8;
    }

    void Store32(uint addr, uint value) {
        if (addr < MIN_VALID_MEMORY) ThrowAbortException("NULL WRITE 32");
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

    // Based on JagDoom code, as we don't have 64-bit multiply in ZScript.
    uint func_FixedMul(int a, int b) {
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
    uint func_FixedDiv(int a, int b) {
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

    void Unreachable() {
        ThrowAbortException("Reached unreachable code");
    }

    void func_I_RV_EndFrame(uint screen) {
        for (uint y = 0; y < 200; y++) {
            for (uint x = 0; x < 320; x++) {
                canvas.Clear(x, y, x + 1, y + 1, palette[memory[screen++]]);
            }
        }
    }

    void func_I_RV_SetPalette(uint addr) {
        for (uint i = 0; i < 256; i++) {
            let r = memory[addr++];
            let g = memory[addr++];
            let b = memory[addr++];
            palette[i] = Color(r, g, b);
        }
    }

    uint func_I_RV_NextEvent(uint ev) {
        if (events.Size() != 0) {
            let event = events[0];
            events.Delete(0);

            Store32(ev, event.type);
            ev += 4;
            Store32(ev, event.data1);
            ev += 4;
            Store32(ev, event.data2);
            ev += 4;
            Store32(ev, event.data3);

            return 1;
        } else {
            return 0;
        }
    }

    uint func_I_RV_GetNumLumps() {
        return lumps.Size();
    }

    void func_I_RV_DumpLumps(uint addr) {
        let numlumps = lumps.Size();
        for (let i = 0; i < numlumps; i++) {
            let name = Wads.GetLumpName(lumps[i]);
            uint j;
            for (j = 0; j < name.Length() && j < 8; j++) {
                memory[addr++] = name.ByteAt(j);
            }
            for (; j < 8; j++) {
                memory[addr++] = 0;
            }
            Store32(addr, Wads.ReadLump(lumps[i]).Length());
            addr += 12;
        }
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

    void func_I_RV_SetMusic(uint name, uint looping) {
        String str = "d_";
        while (memory[name])
            str = String.Format("%s%c", str, memory[name++]);
        S_ChangeMusic(str, 0, !!looping);
    }

    void func_I_RV_PlaySound(uint channel, uint id, uint vol, uint sep) {
        let c = channels[channel];
        c.SetOrigin((pos.x + 160.0 * ((sep - 127.0) / 254.0), pos.y, pos.z), true);
        c.A_StartSound(S_sfx.names[id], CHAN_AUTO, CHANF_DEFAULT, vol / 127.0);
    }

    void func_I_RV_UpdateSound(uint channel, uint vol, uint sep) {
        let c = channels[channel];
        c.SetOrigin((pos.x + 160.0 * ((sep - 127.0) / 254.0), pos.y, pos.z), true);
    }

    void func_I_RV_StopSound(uint channel) {
        let c = channels[channel];
        c.A_StopSound();
    }

    uint func_I_RV_SoundIsPlaying(uint channel) {
        let c = channels[channel];
        return !!c.IsActorPlayingSound(-1);
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

    void InvestigateHeap() {
        uint s = stack;
        uint t = Alloca(4, 4);
        uint b = func_I_ZoneBase(t) + 4;
        uint a = b;
        uint blocks = 0;
        Map<uint, uint> blockThing;
        do {
            if (blockThing.CheckKey(a)) {
                ThrowAbortException("Stuck in a loop");
            }
            if (a + Load32(a) != Load32(a + 16)) {
                ThrowAbortException("Bad size/next link");
            }
            blockThing.InsertNew(a);
            blocks++;
            // Console.Printf("Block @ %u", a);
            a = Load32(a + 16);
        } while (a != b);
        stack = s;
        Console.Printf("%u blocks exist", blocks);
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
            TNT1 A 1 func_D_RunFrame;
            wait;
	}
}
