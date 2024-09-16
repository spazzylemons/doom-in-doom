version "4.12.2"

#include "DoomInDoom/VAList.zs"
#include "DoomInDoom/DoomInDoom.zs"

#include "DoomInDoom/d_event.zs"
#include "DoomInDoom/doomkeys.zs"
#include "DoomInDoom/i_sound.zs"
#include "DoomInDoom/i_video.zs"
#include "DoomInDoom/m_fixed.zs"
#include "DoomInDoom/sounds.zs"

#include "DoomInDoom/code.zs"

class InputListener : EventHandler {
    Map<String, int> cCmdMapping;

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
        cCmdMapping.Insert("spynext",         CCMD_SPYNEXT);
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
    }

    override bool InputProcess(InputEvent e) {
        // TODO blocks most inputs, making gzdoom itself barely usable.
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
            let data1 = 0; // TODO!!
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

        // Find game and forward event.
        let iterator = ThinkerIterator.Create('DoomInDoom');
        DoomInDoom d;
        while ((d = DoomInDoom(iterator.Next()))) {
            d.AddEvent(ev);
        }
    }
}
