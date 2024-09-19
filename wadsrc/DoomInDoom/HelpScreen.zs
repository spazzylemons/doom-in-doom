class DoomInDoomHelpScreen : EventHandler {
    private ui bool loaded;
    private ui HUDFont titleFont;
    private ui HUDFont textFont;

    private bool dismissed;

    private ui void Init() {
        titleFont = HUDFont.Create('BIGUPPER');
        textFont = HUDFont.Create('SMALLFONT');
    }

    private static clearscope String GetSpyBinds() {
        Array<int> spyBinds;
        bindings.GetAllKeysForCommand(spyBinds, "spynext");
        if (spyBinds.Size() == 0) {
            return "currently unbound";
        } else {
            return bindings.NameAllKeys(spyBinds);
        }
    }

    private ui void DrawHelpText() {
        let spyName = Stringtable.Localize("$CNTRLMNU_COOPSPY");
        let spyBinds = GetSpyBinds();

        statusBar.Fill(
            Color(200, 0, 0, 0),
            -20,
            -100,
            360,
            200,
            BaseStatusBar.DI_SCREEN_CENTER
        );

        statusBar.DrawString(
            titleFont,
            "Welcome to Doom in Doom!",
            (0, -80),
            BaseStatusBar.DI_SCREEN_CENTER
        );

        statusBar.DrawString(
            textFont,
            "Your GZDoom keybindings will be mapped to the matching controls. Since Escape is bound to GZDoom's pause menu, Delete is used instead to open/close the menu in the nested Doom instance.\n\n" ..
            "If you want to look around the map, press the '" .. spyName .. "' key (" .. spyBinds .. ") to switch to controlling GZDoom. Press it again to switch back.\n\n" ..
            "Press any key to dismiss this message.",
            (0, -48),
            BaseStatusBar.DI_SCREEN_CENTER,
            Font.CR_WHITE,
            1,
            320,
            2
        );
    }

    override void WorldTick() {
        if (dismissed && !bDESTROYED) {
            Destroy();
        }
    }

    override void RenderOverlay(RenderEvent e) {
        if (!loaded) {
            Init();
            loaded = true;
        }

        DrawHelpText();
    }

    override bool InputProcess(InputEvent e) {
        if (e.type == InputEvent.Type_KeyDown) {
            EventHandler.SendNetworkEvent("doomindoom:dismisshelp");
        }

        return false;
    }

    override void NetworkProcess(ConsoleEvent e) {
        if (e.Name == "doomindoom:dismisshelp") {
            dismissed = true;
        }
    }
}
