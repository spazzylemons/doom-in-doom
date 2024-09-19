/**
 * DoomInDoom - Doom compiled to ZScript
 * Copyright (C) 2024 spazzylemons
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

    static const Sound sfxlist[] = {
        Sound("none"),
        Sound("weapons/pistol"),
        Sound("weapons/shotgf"),
        Sound("weapons/shotgr"),
        Sound("weapons/sshotf"),
        Sound("weapons/sshoto"),
        Sound("weapons/sshotc"),
        Sound("weapons/sshotl"),
        Sound("weapons/plasmaf"),
        Sound("weapons/bfgf"),
        Sound("weapons/sawup"),
        Sound("weapons/sawidle"),
        Sound("weapons/sawfull"),
        Sound("weapons/sawhit"),
        Sound("weapons/rocklf"),
        Sound("weapons/bfgx"),
        Sound("fatso/attack"),
        Sound("weapons/plasmax"),
        Sound("plats/pt1_strt"),
        Sound("plats/pt1_stop"),
        Sound("doors/dr1_open"),
        Sound("doors/dr1_clos"),
        Sound("plats/pt1_mid"),
        Sound("switches/normbutn"),
        Sound("switches/exitbutn"),
        Sound("*pain100"),
        Sound("demon/pain"),
        Sound("grunt/pain"),
        Sound("vile/pain"),
        Sound("fatso/pain"),
        Sound("pain/pain"),
        Sound("misc/gibbed"),
        Sound("misc/i_pkup"),
        Sound("misc/w_pkup"),
        Sound("menu/invalid"),
        Sound("misc/teleport"),
        Sound("grunt/sight1"),
        Sound("grunt/sight2"),
        Sound("grunt/sight3"),
        Sound("imp/sight1"),
        Sound("imp/sight2"),
        Sound("demon/sight"),
        Sound("caco/sight"),
        Sound("baron/sight"),
        Sound("cyber/sight"),
        Sound("spider/sight"),
        Sound("baby/sight"),
        Sound("knight/sight"),
        Sound("vile/sight"),
        Sound("fatso/sight"),
        Sound("pain/sight"),
        Sound("skull/melee"),
        Sound("demon/melee"),
        Sound("skeleton/melee"),
        Sound("vile/start"),
        Sound("imp/melee"),
        Sound("skeleton/swing"),
        Sound("*death"),
        Sound("*xdeath"),
        Sound("grunt/death1"),
        Sound("grunt/death2"),
        Sound("grunt/death3"),
        Sound("imp/death1"),
        Sound("imp/death2"),
        Sound("demon/death"),
        Sound("caco/death"),
        Sound("misc/unused"),
        Sound("baron/death"),
        Sound("cyber/death"),
        Sound("spider/death"),
        Sound("baby/death"),
        Sound("vile/death"),
        Sound("knight/death"),
        Sound("pain/death"),
        Sound("skeleton/death"),
        Sound("grunt/active"),
        Sound("imp/active"),
        Sound("demon/active"),
        Sound("baby/active"),
        Sound("baby/walk"),
        Sound("vile/active"),
        Sound("*usefail"),
        Sound("weapons/rocklx"),
        Sound("*fist"),
        Sound("cyber/hoof"),
        Sound("spider/walk"),
        Sound("weapons/chngun"),
        Sound("misc/chat2"),
        Sound("doors/dr2_open"),
        Sound("doors/dr2_clos"),
        Sound("misc/spawn"),
        Sound("vile/firecrkl"),
        Sound("vile/firestrt"),
        Sound("misc/p_pkup"),
        Sound("brain/spit"),
        Sound("brain/cube"),
        Sound("brain/sight"),
        Sound("brain/pain"),
        Sound("brain/death"),
        Sound("fatso/raiseguns"),
        Sound("fatso/death"),
        Sound("wolfss/sight"),
        Sound("wolfss/death"),
        Sound("keen/pain"),
        Sound("keen/death"),
        Sound("skeleton/active"),
        Sound("skeleton/sight"),
        Sound("skeleton/attack"),
        Sound("misc/chat")
    };

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
        c.A_StartSound(sfxlist[id], CHAN_BODY, CHANF_DEFAULT, vol / 127.0);
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
