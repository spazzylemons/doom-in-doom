//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	The not so system specific sound interface.
//


#ifndef __I_SOUND__
#define __I_SOUND__

#include "doomtype.h"
#include "d_mode.h"

// so that the individual game logic and sound driver code agree
#define NORM_PITCH 127

//
// SoundFX struct.
//
typedef struct sfxinfo_struct	sfxinfo_t;

struct sfxinfo_struct
{
    // Sfx priority
    int priority;

    // referenced sound if a link
    sfxinfo_t *link;

    // pitch if a link (Doom), whether to pitch-shift (Hexen)
    int pitch;

    // volume if a link
    int volume;

    // this is checked every second to see if sound
    // can be thrown out (if 0, then decrement, if -1,
    // then throw out, if > 0, then it is in use)
    int usefulness;
};

//
// MusicInfo struct.
//
typedef struct
{
    // up to 6-character name
    const char *name;

} musicinfo_t;

void I_InitSound(GameMission_t mission);
void I_ShutdownSound(void);
int I_GetSfxLumpNum(sfxinfo_t *sfxinfo);
void I_UpdateSound(void);
void I_UpdateSoundParams(int channel, int vol, int sep);
void I_StartSound(int id, int channel, int vol, int sep, int pitch);
void I_StopSound(int channel);
boolean I_SoundIsPlaying(int channel);
void I_PrecacheSounds(sfxinfo_t *sounds, int num_sounds);

void I_InitMusic(void);
void I_ShutdownMusic(void);
void I_SetMusicVolume(int volume);
void I_PauseSong(void);
void I_ResumeSong(void);
void I_PlaySong(const char *name, boolean looping);
void I_StopSong(void);

extern int snd_pitchshift;

void I_BindSoundVariables(void);

#endif
