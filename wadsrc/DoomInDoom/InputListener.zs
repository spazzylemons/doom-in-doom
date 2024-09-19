class DoomInDoomInputListener : EventHandler {
    private Map<String, int> cCmdMapping;
    private Map<int, int> scanCodeMapping;

    private bool captureInputs;
    private DoomInDoom did;

    clearscope int RemapScancode(InputEvent e) {
        if (e.keyChar >= 0x20 && e.keyChar <= 0x7e) {
            // If character is ASCII, use that.
            return e.keyChar;
        } else {
            // Otherwise, use mapping from GZDoom scancode to Doom scancode.
            return scanCodeMapping.GetIfExists(e.keyScan);
        }
    }

    clearscope int, int GetCCmdType(InputEvent e) {
        int c1, c2;
        bool ok;

        // Try regular bindings, followed by automap bindings.
        [c1, ok] = cCmdMapping.CheckValue(bindings.GetBinding(e.KeyScan));
        if (!ok) c1 = -1;
        [c2, ok] = cCmdMapping.CheckValue(automapBindings.GetBinding(e.KeyScan));
        if (!ok) c2 = -1;

        return c1, c2;
    }

    override void OnRegister() {
        cCmdMapping.Insert("+attack",         CCMD_ATTACK);
        cCmdMapping.Insert("+use",            CCMD_USE);
        cCmdMapping.Insert("+forward",        CCMD_FORWARD);
        cCmdMapping.Insert("+back",           CCMD_BACK);
        cCmdMapping.Insert("+moveleft",       CCMD_MOVELEFT);
        cCmdMapping.Insert("+moveright",      CCMD_MOVERIGHT);
        cCmdMapping.Insert("+left",           CCMD_LEFT);
        cCmdMapping.Insert("+right",          CCMD_RIGHT);
        cCmdMapping.Insert("+speed",          CCMD_SPEED);
        cCmdMapping.Insert("+strafe",         CCMD_STRAFE);
        cCmdMapping.Insert("weapnext",        CCMD_WEAPNEXT);
        cCmdMapping.Insert("weapprev",        CCMD_WEAPPREV);
        cCmdMapping.Insert("slot 1",          CCMD_SLOT_1);
        cCmdMapping.Insert("slot 2",          CCMD_SLOT_2);
        cCmdMapping.Insert("slot 3",          CCMD_SLOT_3);
        cCmdMapping.Insert("slot 4",          CCMD_SLOT_4);
        cCmdMapping.Insert("slot 5",          CCMD_SLOT_5);
        cCmdMapping.Insert("slot 6",          CCMD_SLOT_6);
        cCmdMapping.Insert("slot 7",          CCMD_SLOT_7);
        cCmdMapping.Insert("slot 8",          CCMD_SLOT_8);
        cCmdMapping.Insert("togglemap",       CCMD_TOGGLEMAP);
        cCmdMapping.Insert("+am_panleft",     CCMD_AM_PANLEFT);
        cCmdMapping.Insert("+am_panright",    CCMD_AM_PANRIGHT);
        cCmdMapping.Insert("+am_panup",       CCMD_AM_PANUP);
        cCmdMapping.Insert("+am_pandown",     CCMD_AM_PANDOWN);
        cCmdMapping.Insert("+am_zoomin",      CCMD_AM_ZOOMIN);
        cCmdMapping.Insert("+am_zoomout",     CCMD_AM_ZOOMOUT);
        cCmdMapping.Insert("am_gobig",        CCMD_AM_GOBIG);
        cCmdMapping.Insert("am_togglefollow", CCMD_AM_TOGGLEFOLLOW);
        cCmdMapping.Insert("am_togglegrid",   CCMD_AM_TOGGLEGRID);
        cCmdMapping.Insert("am_setmark",      CCMD_AM_SETMARK);
        cCmdMapping.Insert("am_clearmarks",   CCMD_AM_CLEARMARKS);
        cCmdMapping.Insert("sizeup",          CCMD_SIZEUP);
        cCmdMapping.Insert("sizedown",        CCMD_SIZEDOWN);
        cCmdMapping.Insert("menu_main",       CCMD_MENU_MAIN);
        cCmdMapping.Insert("menu_help",       CCMD_MENU_HELP);
        cCmdMapping.Insert("menu_save",       CCMD_MENU_SAVE);
        cCmdMapping.Insert("menu_load",       CCMD_MENU_LOAD);
        cCmdMapping.Insert("menu_options",    CCMD_MENU_OPTIONS);
        cCmdMapping.Insert("quicksave",       CCMD_QUICKSAVE);
        cCmdMapping.Insert("menu_endgame",    CCMD_ENDGAME);
        cCmdMapping.Insert("togglemessages",  CCMD_TOGGLEMESSAGES);
        cCmdMapping.Insert("quickload",       CCMD_QUICKLOAD);
        cCmdMapping.Insert("menu_quit",       CCMD_MENU_QUIT);
        cCmdMapping.Insert("bumpgamma",       CCMD_BUMPGAMMA);

        scanCodeMapping.Insert(InputEvent.Key_Pause,       KEY_PAUSE);
        scanCodeMapping.Insert(InputEvent.Key_RightArrow,  KEY_RIGHTARROW);
        scanCodeMapping.Insert(InputEvent.Key_LeftArrow,   KEY_LEFTARROW);
        scanCodeMapping.Insert(InputEvent.Key_UpArrow,     KEY_UPARROW);
        scanCodeMapping.Insert(InputEvent.Key_DownArrow,   KEY_DOWNARROW);
        scanCodeMapping.Insert(InputEvent.Key_Escape,      KEY_ESCAPE);
        scanCodeMapping.Insert(InputEvent.Key_Enter,       KEY_ENTER);
        scanCodeMapping.Insert(InputEvent.Key_F1,          KEY_F1);
        scanCodeMapping.Insert(InputEvent.Key_F2,          KEY_F2);
        scanCodeMapping.Insert(InputEvent.Key_F3,          KEY_F3);
        scanCodeMapping.Insert(InputEvent.Key_F4,          KEY_F4);
        scanCodeMapping.Insert(InputEvent.Key_F5,          KEY_F5);
        scanCodeMapping.Insert(InputEvent.Key_F6,          KEY_F6);
        scanCodeMapping.Insert(InputEvent.Key_F7,          KEY_F7);
        scanCodeMapping.Insert(InputEvent.Key_F8,          KEY_F8);
        scanCodeMapping.Insert(InputEvent.Key_F9,          KEY_F9);
        scanCodeMapping.Insert(InputEvent.Key_F10,         KEY_F10);
        scanCodeMapping.Insert(InputEvent.Key_F11,         KEY_F11);
        scanCodeMapping.Insert(InputEvent.Key_F12,         KEY_F12);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_1,      KEYP_1);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_2,      KEYP_2);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_3,      KEYP_3);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_4,      KEYP_4);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_5,      KEYP_5);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_6,      KEYP_6);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_7,      KEYP_7);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_8,      KEYP_8);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_9,      KEYP_9);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_0,      KEYP_0);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_Minus,  KEYP_MINUS);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_Plus,   KEYP_PLUS);
        scanCodeMapping.Insert(InputEvent.KEY_kpad_Period, KEYP_PERIOD);
        scanCodeMapping.Insert(InputEvent.Key_Backspace,   KEY_BACKSPACE);
        scanCodeMapping.Insert(InputEvent.Key_LShift,      KEY_RSHIFT);
        scanCodeMapping.Insert(InputEvent.Key_LCtrl,       KEY_RCTRL);
        scanCodeMapping.Insert(InputEvent.Key_LAlt,        KEY_RALT);
        scanCodeMapping.Insert(InputEvent.Key_RShift,      KEY_RSHIFT);
        scanCodeMapping.Insert(InputEvent.Key_RCtrl,       KEY_RCTRL);
        scanCodeMapping.Insert(InputEvent.Key_RAlt,        KEY_RALT);
        scanCodeMapping.Insert(InputEvent.Key_Ins,         KEY_INS);
        scanCodeMapping.Insert(InputEvent.Key_Del,         KEY_DEL);
        scanCodeMapping.Insert(InputEvent.Key_End,         KEY_END);
        scanCodeMapping.Insert(InputEvent.Key_Home,        KEY_HOME);
        scanCodeMapping.Insert(InputEvent.Key_PgUp,        KEY_PGUP);
        scanCodeMapping.Insert(InputEvent.Key_PgDn,        KEY_PGDN);

    }

    override void WorldLoaded(WorldEvent e) {
        let iterator = ThinkerIterator.Create('DoomInDoom');
        did = DoomInDoom(iterator.Next());
        captureInputs = true;
    }

    override bool InputProcess(InputEvent e) {
        if (did == null)
            return false;

        if (e.type == InputEvent.Type_KeyDown && bindings.GetBinding(e.KeyScan) == "spynext") {
            EventHandler.SendNetworkEvent("doomindoom:togglecapture");
            return false;
        }

        if (!captureInputs) {
            return false;
        }

        if (e.type == InputEvent.Type_KeyDown) {
            // TODO handle shift for data3.
            let data1 = RemapScancode(e);
            let data2 = e.keyChar;
            let data3 = e.keyChar; // TODO

            EventHandler.SendNetworkEvent("doomindoom:keydown", data1, data2, data3);

            let [c1, c2] = GetCCmdType(e);
            if (c1 >= 0)
                EventHandler.SendNetworkEvent("doomindoom:buttondown", c1);
            if (c2 >= 0)
                EventHandler.SendNetworkEvent("doomindoom:buttondown", c2);

            return true;
        } else if (e.type == InputEvent.Type_KeyUp) {
            let data1 = RemapScancode(e);

            EventHandler.SendNetworkEvent("doomindoom:keyup", data1);

            let [c1, c2] = GetCCmdType(e);
            if (c1 >= 0)
                EventHandler.SendNetworkEvent("doomindoom:buttonup", c1);
            if (c2 >= 0)
                EventHandler.SendNetworkEvent("doomindoom:buttonup", c2);

            return true;
        } else if (e.type == InputEvent.Type_Mouse) {
            let data1 = 0;
            let data2 = e.mouseX;
            let data3 = e.mouseY;

            EventHandler.SendNetworkEvent("doomindoom:mouse", data1, data2, data3);
            return true;
        } else {
            return false;
        }
    }

    override void NetworkProcess(ConsoleEvent e) {
        if (e.IsManual)
            return;

        if (e.Name == "doomindoom:togglecapture") {
            captureInputs = !captureInputs;

            let msg = captureInputs ? "Controlling Doom in Doom." : "Controlling GZDoom.";
            Console.MidPrint(Font.FindFont('BIGFONT'), msg);
            return;
        }

        let ev = new('event_t');
        if (e.Name == "doomindoom:keydown") {
            ev.type = ev_keydown;
        } else if (e.Name == "doomindoom:keyup") {
            ev.type = ev_keyup;
        } else if (e.Name == "doomindoom:mouse") {
            ev.type = ev_mouse;
        } else if (e.Name == "doomindoom:buttondown") {
            ev.type = ev_buttondown;
        } else if (e.Name == "doomindoom:buttonup") {
            ev.type = ev_buttonup;
        } else {
            return;
        }

        ev.data1 = e.Args[0];
        ev.data2 = e.Args[1];
        ev.data3 = e.Args[2];
        did.AddEvent(ev);
    }
}
