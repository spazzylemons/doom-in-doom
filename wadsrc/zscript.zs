version "4.12.2"

#include "DoomInDoom/d_event.zs"
#include "DoomInDoom/doomkeys.zs"
#include "DoomInDoom/sounds.zs"

class DoomSFX : Actor {
    default {
        Height 1;
        Radius 1;

        +NOCLIP
    }

    states {
        Spawn:
            TNT1 A -1;
            stop;
    }
}

class RV32Doom : Actor {
    uint saved_regs[32];
    uint saved_pc;

    bool restarted;
    String linebuffer;

    Array<int> lumps;
    Map<int, sound> sounds;

    Array<event_t> events;

    uint ticCount;

    Canvas canvas;

    Actor channels[8];

    Color palette[256];

    uint8 memory[16777216];

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
    }

    int Load1Signed(uint addr) {
        uint a = memory[addr];
        return ((a + 128) & 0xff) - 128;
    }

    uint Load2(uint addr) {
        uint a = memory[addr++];
        uint b = memory[addr];
        return a | (b << 8);
    }

    int Load2Signed(uint addr) {
        uint a = memory[addr++];
        uint b = memory[addr];
        return (((a | (b << 8)) + 32768) & 0xffff) - 32768;
    }

    uint Load4(uint addr) {
        uint a = memory[addr++];
        uint b = memory[addr++];
        uint c = memory[addr++];
        uint d = memory[addr];
        return a | (b << 8) | (c << 16) | (d << 24);
    }

    void Store2(uint addr, uint value) {
        memory[addr++] = value;
        memory[addr] = value >> 8;
    }

    void Store4(uint addr, uint value) {
        memory[addr++] = value;
        memory[addr++] = value >> 8;
        memory[addr++] = value >> 16;
        memory[addr] = value >> 24;
    }

    uint LongMultiply(uint x, uint y, uint xor) {
        uint xl = x & 0xffff;
        uint xh = x >> 16;

        uint yl = y & 0xffff;
        uint yh = y >> 16;

        uint lo = 0;
        uint hi = 0;
        uint last = 0;
        uint acc = 0;

        last = lo = xl * yl;
        acc = xh * yl + xl * yh;
        lo += acc << 16;
        if (lo < last) ++hi;
        hi += acc >> 16;
        hi += xh * yh;

        if (xor) {
            hi = -hi;
            if (lo) --hi;
        }

        return hi;
    }

    void Run() {
        if (canvas == null) ThrowAbortException("holy crap lois where am i");

        if (restarted) {
            Reset();
        }

        uint pc = saved_pc;
        uint regs[32];

        for (uint i = 0; i < 32; i++)
            regs[i] = saved_regs[i];

        let running = true;

        while (running) {
            uint rval = 0;
            uint ir = Load4(pc);
            uint rdid = (ir >> 7) & 0x1f;

            uint imm = ir >> 20;
            uint immse = imm | ((imm & 0x800) ? 0xfffff000 : 0);

            uint op = (ir >> 12) & 7;

            switch (ir & 0x7f) {
                case 0x37:
                    rval = (ir & 0xfffff000);
                    break;

                case 0x17:
                    rval = pc + (ir & 0xfffff000);
                    break;

                case 0x6f: {
                    uint reladdy
                        = ((ir & 0x80000000) >> 11)
                        | ((ir & 0x7fe00000) >> 20)
                        | ((ir & 0x00100000) >> 9)
                        | ((ir & 0x000ff000));

                    if (reladdy & 0x00100000)
                        reladdy |= 0xffe00000;

                    rval = pc + 4;
                    pc += reladdy - 4;
                    break;
                }

                case 0x67: {
                    uint rs1 = regs[(ir >> 15) & 0x1f];
                    rval = pc + 4;
                    pc = ((rs1 + immse) & 0xfffffffe) - 4;
                    break;
                }

                case 0x63: {
                    uint rs1 = regs[(ir >> 15) & 0x1f];
                    uint rs2 = regs[(ir >> 20) & 0x1f];
                    uint immm4 = ((ir & 0xf00) >> 7) | ((ir & 0x7e000000) >> 20) | ((ir & 0x80) << 4) | ((ir >> 31) << 12);
                    if (immm4 & 0x1000) immm4 |= 0xffffe000;
                    immm4 += pc - 4;
                    rdid = 0;
                    switch (op) {
                        case 0:
                            if (rs1 == rs2)
                                pc = immm4;
                            break;
                        case 1:
                            if (rs1 != rs2)
                                pc = immm4;
                            break;
                        case 4:
                            if (int(rs1) < int(rs2))
                                pc = immm4;
                            break;
                        case 5:
                            if (int(rs1) >= int(rs2))
                                pc = immm4;
                            break;
                        case 6:
                            if (rs1 < rs2)
                                pc = immm4;
                            break;
                        case 7:
                            if (rs1 >= rs2)
                                pc = immm4;
                            break;
                        default:
                            ThrowAbortException("Illegal instruction");
                    }
                    break;
                }
                case 0x03: {
                    uint rs1 = regs[(ir >> 15) & 0x1f];
                    uint rsval = rs1 + immse;
                    switch (op) {
                        case 0: rval = Load1Signed( rsval ); break;
                        case 1: rval = Load2Signed( rsval ); break;
                        case 2: rval = Load4( rsval ); break;
                        case 4: rval = memory[rsval]; break;
                        case 5: rval = Load2( rsval ); break;
                        default:
                            ThrowAbortException("Illegal instruction");
                    }
                    break;
                }
                case 0x23: {
                    uint rs1 = regs[(ir >> 15) & 0x1f];
                    uint rs2 = regs[(ir >> 20) & 0x1f];
                    uint addy = rdid | ((ir & 0xfe000000) >> 20);
                    if (addy & 0x800) addy |= 0xfffff000;
                    addy += rs1;
                    rdid = 0;

                    switch (op) {
                        case 0:
                            memory[addy] = rs2;
                            break;
                        case 1:
                            Store2(addy, rs2);
                            break;
                        case 2:
                            Store4(addy, rs2);
                            break;
                        default:
                            ThrowAbortException("Illegal instruction");
                    }
                    break;
                }
                case 0x13: case 0x33: {
                    uint rs1 = regs[(ir >> 15) & 0x1f];
                    uint rs2 = regs[(ir >> 20) & 0x1f];
                    bool is_reg = (ir & 0x20) != 0;

                    if (!is_reg) {
                        rs2 = ir >> 20;
                        rs2 = immse;
                    }

                    if (is_reg && (ir & 0x02000000)) {
                        switch (op) {
                            case 0: rval = rs1 * rs2; break;
                            case 1: {
                                let x = rs1;
                                let y = rs2;
                                let xor = 0;
                                if (x & 0x80000000) {
                                    x = -int(x);
                                    xor = ~xor;
                                }
                                if (y & 0x80000000) {
                                    y = -int(y);
                                    xor = ~xor;
                                }
                                rval = LongMultiply(x, y, xor);
                                break;
                            }
                            case 2: {
                                let x = rs1;
                                let y = rs2;
                                let xor = 0;
                                if (x & 0x80000000) {
                                    x = -int(x);
                                    xor = ~xor;
                                }
                                rval = LongMultiply(x, y, xor);
                                break;
                            }
                            case 3: {
                                rval = LongMultiply(rs1, rs2, 0);
                                break;
                            }
                            case 4:
                                if (rs2 == 0)
                                    rval = -1;
                                else if (rs1 == 0x80000000 && rs2 == 0xffffffff)
                                    rval = rs1;
                                else
                                    rval = int(rs1) / int(rs2);
                                break;

                            case 5:
                                if (rs2 == 0)
                                    rval = 0xffffffff;
                                else
                                    rval = rs1 / rs2;
                                break;
                            case 6:
                                if (rs2 == 0)
                                    rval = rs1;
                                else if (rs1 == 0x80000000 && rs2 == 0xffffffff)
                                    rval = 0;
                                else
                                    rval = int(rs1) % int(rs2);
                                break;
                            case 7:
                                if (rs2 == 0)
                                    rval = rs1;
                                else
                                    rval = rs1 % rs2;
                                break;
                        }
                    } else {
                        switch (op) {
                            case 0:
                                rval = (is_reg && (ir & 0x40000000)) ? (rs1 - rs2) : (rs1 + rs2);
                                break; 
                            case 1:
                                rval = rs1 << (rs2 & 0x1f);
                                break;
                            case 2:
                                rval = int(rs1) < int(rs2);
                                break;
                            case 3:
                                rval = rs1 < rs2;
                                break;
                            case 4:
                                rval = rs1 ^ rs2;
                                break;
                            case 5:
                                rval = (ir & 0x40000000) ? (int(rs1) >> (rs2 & 0x1f)) : (rs1 >> (rs2 & 0x1f));
                                break;
                            case 6:
                                rval = rs1 | rs2;
                                break;
                            case 7:
                                rval = rs1 & rs2;
                                break;
                        }
                    }
                    break;
                }
                case 0x73: {
                    uint funct7 = ir >> 25;
                    if (op == 0) {
                        switch (regs[10]) {
                            case 0: {
                                running = false;
                                let addr = regs[11];
                                for (uint y = 0; y < 200; y++) {
                                    for (uint x = 0; x < 320; x++) {
                                        canvas.Clear(x, y, x + 1, y + 1, palette[memory[addr++]]);
                                    }
                                }
                                break;
                            }
                            case 1: {
                                let addr = regs[11];
                                for (uint i = 0; i < 256; i++) {
                                    let r = memory[addr++];
                                    let g = memory[addr++];
                                    let b = memory[addr++];
                                    palette[i] = Color(r, g, b);
                                }
                                break;
                            }
                            case 2:
                                if (events.Size() != 0) {
                                    let addr = regs[11];

                                    let event = events[0];
                                    events.Delete(0);

                                    Store4(addr, event.type);
                                    addr += 4;
                                    Store4(addr, event.data1);
                                    addr += 4;
                                    Store4(addr, event.data2);
                                    addr += 4;
                                    Store4(addr, event.data3);

                                    regs[10] = 1;
                                } else {
                                    regs[10] = 0;
                                }
                                break;
                            case 3:
                                regs[10] = lumps.Size();
                                break;
                            case 4: {
                                let numlumps = lumps.Size();
                                let addr = regs[11];
                                for (let i = 0; i < numlumps; i++) {
                                    let name = Wads.GetLumpName(lumps[i]);
                                    uint j;
                                    for (j = 0; j < name.Length() && j < 8; j++) {
                                        memory[addr++] = name.ByteAt(j);
                                    }
                                    for (; j < 8; j++) {
                                        memory[addr++] = 0;
                                    }
                                    Store4(addr, Wads.ReadLump(lumps[i]).Length());
                                    addr += 12;
                                }
                                break;
                            }
                            case 5:
                                let addr = regs[12];
                                let data = Wads.ReadLump(lumps[regs[11]]);
                                for (uint i = 0; i < data.Length(); i++) {
                                    memory[addr++] = data.ByteAt(i);
                                }
                                break;
                            case 6:
                                regs[10] = 0;
                                break;
                            case 7:
                                break;
                            case 8:
                                regs[10] = 0;
                                break;
                            case 9:
                                regs[10] = 0;
                                break;
                            case 10:
                                break;
                            case 11:
                                break;
                            case 12:
                                break;
                            /*case 3: {
                                let addr = regs[12];
                                String str = "";
                                while (memory[addr]) {
                                    str = String.Format("%s%c", str, memory[addr++]);
                                }
                                let snd = Sound(str);
                                sounds.Insert(regs[11], snd);
                                break;
                            }
                            case 4:
                                S_StartSound(sounds.Get(regs[11]), CHAN_AUTO);
                                break;
                            case 5: {
                                let n = Wads.GetLumpName(lumps[regs[11]]);
                                S_ChangeMusic(n);
                                break;
                            }*/
                            case 13:
                                running = false;
                                restarted = true;
                                break;
                            case 14: {
                                let addr = regs[11];
                                String str = "d_";
                                while (memory[addr])
                                    str = String.Format("%s%c", str, memory[addr++]);
                                S_ChangeMusic(str, 0, !!regs[12]);
                                break;
                            }
                            case 15: {
                                let channel = regs[11];
                                let id = regs[12];
                                let vol = regs[13];
                                let sep = regs[14];

                                let c = channels[channel];
                                c.SetOrigin((pos.x + 160.0 * ((sep - 127.0) / 254.0), pos.y, pos.z), true);
                                c.A_StartSound(S_sfx.names[id], CHAN_AUTO, CHANF_DEFAULT, vol / 127.0);
                                break;
                            }
                            case 16: {
                                let channel = regs[11];
                                let vol = regs[12];
                                let sep = regs[13];

                                let c = channels[channel];
                                c.SetOrigin((pos.x + 160.0 * ((sep - 127.0) / 254.0), pos.y, pos.z), true);
                                break;
                            }
                            case 17: {
                                let channel = regs[11];

                                let c = channels[channel];
                                c.A_StopSound();
                                break;
                            }
                            case 18: {
                                let channel = regs[11];

                                let c = channels[channel];
                                regs[10] = c.IsActorPlayingSound(-1);
                                break;
                            }
                            case 19: {
                                uint c = regs[11] & 0xff;
                                if (c == 10) {
                                    Console.Printf("%s", linebuffer);
                                    linebuffer = "";
                                } else if (c >= 0x20 && c <= 0x7e) {
                                    linebuffer = String.Format("%s%c", linebuffer, c);
                                }
                                break;
                            }
                            default:
                                ThrowAbortException("Unhandled ECALL");
                        }
                    } else if (op == 4 && funct7 == 0x41) {
                        // assume MOP.R.R.0
                        // This is dedicated to FixedDiv for speeding up
                        // fixed-point division.
                        int a = regs[(ir >> 15) & 0x1f];
                        int b = regs[(ir >> 20) & 0x1f];

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
                            rval = sign < 0 ? 0x80000000 : 0x7fffffff;
                        } else {
                            uint bit = 0x10000;
                            while (aa > bb) {
                                bb <<= 1;
                                bit <<= 1;
                            }

                            rval = 0;

                            do {
                                if (aa >= bb) {
                                    aa -= bb;
                                    rval |= bit;
                                }
                                aa <<= 1;
                                bit >>= 1;
                            } while (bit && aa);

                            if (sign < 0)
                                rval = -rval;
                        }
                    } else {
                        ThrowAbortException("Illegal instruction");
                    }

                    break;
                }

                default:
                    ThrowAbortException("Illegal instruction");
            }

            if (rdid) {
                regs[rdid] = rval;
            }

            pc += 4;
        }

        ticCount++;

        saved_pc = pc;

        for (uint i = 0; i < 32; i++)
            saved_regs[i] = regs[i];
    }

    void Reset() {
        if (linebuffer != "") {
            Console.Printf("%s", linebuffer);
            linebuffer = "";
        }

        saved_pc = 0;
        restarted = false;
        ticCount = 0;

        let rom = Wads.ReadLump(Wads.CheckNumForFullName("DoomInDoom/DOOM_EXE"));
        uint i;
        for (i = 0; i < rom.Length(); i++) {
            memory[i] = rom.ByteAt(i);
        }
        for (; i < 16777216; i++) {
            memory[i] = 0;
        }

        sounds.Clear();
    }

    void AddEvent(event_t e) {
        events.Push(e);
    }

    default {
        Height 1;
        Radius 1;

        +NOCLIP;
    }

	states {
		Spawn:
            TNT1 A 0;
            TNT1 A 0 Load;
            TNT1 A 0 Reset;
            TNT1 A 1 Run;
            wait;
	}
}

class InputListener : EventHandler {
    static clearscope int RemapScancode(InputEvent e) {
        if (e.keyChar != 0) {
            return e.keyChar;
        } else {
            let c = e.keyScan;
            if (c == InputEvent.Key_Pause) return KEY_PAUSE;
            if (c == InputEvent.Key_RightArrow) return KEY_RIGHTARROW;
            if (c == InputEvent.Key_LeftArrow) return KEY_LEFTARROW;
            if (c == InputEvent.Key_UpArrow) return KEY_UPARROW;
            if (c == InputEvent.Key_DownArrow) return KEY_DOWNARROW;
            if (c == InputEvent.Key_Escape) return KEY_ESCAPE;
            if (c == InputEvent.Key_F1) return KEY_F1;
            if (c == InputEvent.Key_F2) return KEY_F2;
            if (c == InputEvent.Key_F3) return KEY_F3;
            if (c == InputEvent.Key_F4) return KEY_F4;
            if (c == InputEvent.Key_F5) return KEY_F5;
            if (c == InputEvent.Key_F6) return KEY_F6;
            if (c == InputEvent.Key_F7) return KEY_F7;
            if (c == InputEvent.Key_F8) return KEY_F8;
            if (c == InputEvent.Key_F9) return KEY_F9;
            if (c == InputEvent.Key_F10) return KEY_F10;
            if (c == InputEvent.Key_F11) return KEY_F11;
            if (c == InputEvent.Key_F12) return KEY_F12;
            if (c == InputEvent.KEY_kpad_1) return KEYP_1;
            if (c == InputEvent.KEY_kpad_2) return KEYP_2;
            if (c == InputEvent.KEY_kpad_3) return KEYP_3;
            if (c == InputEvent.KEY_kpad_4) return KEYP_4;
            if (c == InputEvent.KEY_kpad_5) return KEYP_5;
            if (c == InputEvent.KEY_kpad_6) return KEYP_6;
            if (c == InputEvent.KEY_kpad_7) return KEYP_7;
            if (c == InputEvent.KEY_kpad_8) return KEYP_8;
            if (c == InputEvent.KEY_kpad_9) return KEYP_9;
            if (c == InputEvent.KEY_kpad_0) return KEYP_0;
            if (c == InputEvent.KEY_kpad_Minus) return KEYP_MINUS;
            if (c == InputEvent.KEY_kpad_Plus) return KEYP_PLUS;
            if (c == InputEvent.KEY_kpad_Period) return KEYP_PERIOD;
            if (c == InputEvent.Key_Backspace) return KEY_BACKSPACE;
            if (c == InputEvent.Key_LShift) return KEY_RSHIFT;
            if (c == InputEvent.Key_LCtrl) return KEY_RCTRL;
            if (c == InputEvent.Key_LAlt) return KEY_RALT;
            if (c == InputEvent.Key_RShift) return KEY_RSHIFT;
            if (c == InputEvent.Key_RCtrl) return KEY_RCTRL;
            if (c == InputEvent.Key_RAlt) return KEY_RALT;
            if (c == InputEvent.Key_Ins) return KEY_INS;
            if (c == InputEvent.Key_Del) return KEY_DEL;
            if (c == InputEvent.Key_End) return KEY_END;
            if (c == InputEvent.Key_Home) return KEY_HOME;
            if (c == InputEvent.Key_PgUp) return KEY_PGUP;
            if (c == InputEvent.Key_PgDn) return KEY_PGDN;
            return 0;
        }
    }

    override bool InputProcess(InputEvent e) {
        // TODO blocks most inputs, making gzdoom itself barely usable.
        if (e.type == InputEvent.Type_KeyDown) {
            // TODO handle shift for data3.
            let data1 = RemapScancode(e);
            let data2 = e.keyChar;
            let data3 = e.keyChar; // TODO

            EventHandler.SendNetworkEvent("rv32doom:keydown", data1, data2, data3);
            return true;
        } else if (e.type == InputEvent.Type_KeyUp) {
            let data1 = RemapScancode(e);

            EventHandler.SendNetworkEvent("rv32doom:keyup", data1);
            return true;
        } else if (e.type == InputEvent.Type_Mouse) {
            let data1 = 0; // TODO!!
            let data2 = e.mouseX;
            let data3 = e.mouseY;

            EventHandler.SendNetworkEvent("rv32doom:mouse", data1, data2, data3);
            return true;
        } else {
            return false;
        }
    }

    override void NetworkProcess(ConsoleEvent e) {
        if (e.IsManual)
            return;

        let ev = new('event_t');

        if (e.Name == "rv32doom:keydown") {
            ev.type = ev_keydown;
        } else if (e.Name == "rv32doom:keyup") {
            ev.type = ev_keyup;
        } else if (e.Name == "rv32doom:mouse") {
            ev.type = ev_mouse;
        } else {
            return;
        }

        ev.data1 = e.Args[0];
        ev.data2 = e.Args[1];
        ev.data3 = e.Args[2];

        // Find all emulators and forward them the event.
        let iterator = ThinkerIterator.Create('RV32Doom');
        RV32Doom emulator;
        while ((emulator = RV32Doom(iterator.Next()))) {
            emulator.AddEvent(ev);
        }
    }
}
