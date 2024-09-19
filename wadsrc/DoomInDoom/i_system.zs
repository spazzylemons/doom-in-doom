const ERROR_MESSAGE_BUF_SIZE = 512;

extend class DoomInDoom {
    private bool alreadyQuitting;

    void func_I_Error(uint error, VAList v) {
        if (alreadyQuitting) {
            ThrowAbortException("Warning: recursive call to I_Error detected.");
        }
        alreadyQuitting = true;

        uint argptr = Alloca(4, 4);
        uint msgbuf = Alloca(ERROR_MESSAGE_BUF_SIZE, 1);
        v.Start(argptr);
        func_M_vsnprintf(msgbuf, ERROR_MESSAGE_BUF_SIZE, error, Load32(argptr));
        v.End(argptr);

        ThrowAbortException(GetString(msgbuf));
    }

    void func_I_Exit() {
        Die(self, self);
    }
}
