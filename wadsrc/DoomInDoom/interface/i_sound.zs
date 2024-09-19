class DoomSoundSource : Actor {
    Vector3 centerPos;

    default {
        Height 1;
        Radius 1;

        +NOCLIP
    }

    states {
        Spawn:
            TNT1 A 0;
            TNT1 A 0 SaveCenterPos;
            TNT1 A -1;
            stop;
    }

    void SaveCenterPos() {
        centerPos = pos;
    }

    void SetPanning(uint sep) {
        SetOrigin((centerPos.x + 160.0 * ((sep - 127.0) / 254.0), centerPos.y, centerPos.z), true);
    }
}

extend class DoomInDoom {
    private DoomSoundSource channels[8];

    static void CheckVolumeSeparation(in out uint vol, in out uint sep) {
        if (int(sep) < 0) {
            sep = 0;
        } else if (int(sep) > 254) {
            sep = 254;
        }

        if (int(vol) < 0) {
            vol = 0;
        } else if (int(vol) > 127) {
            vol = 127;
        }
    }

    void func_I_InitSound(uint mission) {
        for (uint i = 0; i < 8; i++) {
            channels[i] = DoomSoundSource(Actor.Spawn('DoomSoundSource', pos));
        }
    }

    void func_I_PlaySong(uint name, uint looping) {
        String str = "d_" .. GetString(name);
        S_ChangeMusic(str, 0, !!looping);
    }

    void func_I_UpdateSoundParams(uint channel, uint vol, uint sep) {
        CheckVolumeSeparation(vol, sep);

        // TODO update volume.
        let c = channels[channel];
        c.SetPanning(sep);
        c.A_SoundVolume(CHAN_BODY, vol / 127.0);
    }

    void func_I_StartSound(uint id, uint channel, uint vol, uint sep, uint pitch) {
        CheckVolumeSeparation(vol, sep);

        let c = channels[channel];
        c.SetPanning(sep);
        c.A_StartSound(S_sfx.names[id - 1], CHAN_BODY, CHANF_DEFAULT, vol / 127.0);
    }

    void func_I_StopSound(uint channel) {
        let c = channels[channel];
        c.A_StopSound();
    }

    uint func_I_SoundIsPlaying(uint channel) {
        let c = channels[channel];
        return !!c.IsActorPlayingSound(CHAN_BODY);
    }
}
