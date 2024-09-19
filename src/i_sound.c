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
// DESCRIPTION:  none
//

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "doomtype.h"

#include "i_sound.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"
#include "sounds.h"

// Whether to vary the pitch of sound effects
// Each game will set the default differently

int snd_pitchshift = -1;

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void I_ShutdownSound(void)
{
}

int I_GetSfxLumpNum(sfxinfo_t *sfxinfo)
{
    return 0;
}

void I_UpdateSound(void)
{
}

void I_PrecacheSounds(sfxinfo_t *sounds, int num_sounds)
{
}

void I_InitMusic(void)
{
}

void I_ShutdownMusic(void)
{

}

void I_SetMusicVolume(int volume)
{
}

void I_PauseSong(void)
{
}

void I_ResumeSong(void)
{
}

void I_StopSong(void)
{
}

void I_BindSoundVariables(void)
{
    M_BindIntVariable("snd_pitchshift",          &snd_pitchshift);
}

