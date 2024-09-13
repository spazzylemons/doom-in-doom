version "4.12.2"

#include "DoomInDoom/VAList.zs"
#include "DoomInDoom/DoomInDoom.zs"

#include "DoomInDoom/d_event.zs"
#include "DoomInDoom/doomkeys.zs"
#include "DoomInDoom/sounds.zs"

#include "DoomInDoom/code.zs"

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

            EventHandler.SendNetworkEvent("doomindoom:keydown", data1, data2, data3);
            return true;
        } else if (e.type == InputEvent.Type_KeyUp) {
            let data1 = RemapScancode(e);

            EventHandler.SendNetworkEvent("doomindoom:keyup", data1);
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
