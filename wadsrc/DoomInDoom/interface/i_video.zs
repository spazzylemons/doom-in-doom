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
