class VAList play {
    Array<uint> values;
    DoomInDoom d;
    uint testb;

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
        testb = block;
    }

    void End(uint ptr) {
        d.Store32(ptr, 0);
        if (d.stack > testb) {
            ThrowAbortException("STACKFAIL");
        }
    }
}
