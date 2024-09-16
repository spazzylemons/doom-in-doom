//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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

#include <stdio.h>

#include "doomtype.h"
#include "doomkeys.h"

#include "m_config.h"
#include "m_misc.h"

int key_message_refresh = KEY_ENTER;
int key_pause = KEY_PAUSE;
int key_demo_quit = 'q';
int key_spy = KEY_F12;

// Multiplayer chat keys:

int key_multi_msg = 't';
int key_multi_msgplayer[8];

// menu keys:

int key_menu_activate  = KEY_ESCAPE;
int key_menu_up        = KEY_UPARROW;
int key_menu_down      = KEY_DOWNARROW;
int key_menu_left      = KEY_LEFTARROW;
int key_menu_right     = KEY_RIGHTARROW;
int key_menu_back      = KEY_BACKSPACE;
int key_menu_forward   = KEY_ENTER;
int key_menu_confirm   = 'y';
int key_menu_abort     = 'n';
 
// 
// Bind all of the common controls used by Doom and all other games.
//

void M_BindBaseControls(void)
{
    M_BindIntVariable("key_pause",           &key_pause);
    M_BindIntVariable("key_message_refresh", &key_message_refresh);
}

void M_BindHereticControls(void)
{
}

void M_BindHexenControls(void)
{
}

void M_BindStrifeControls(void)
{
    // These are shared with all games, but have different defaults:
    key_message_refresh = '/';
}

void M_BindWeaponControls(void)
{
}

void M_BindMapControls(void)
{
}

void M_BindMenuControls(void)
{
    M_BindIntVariable("key_menu_activate",  &key_menu_activate);
    M_BindIntVariable("key_menu_up",        &key_menu_up);
    M_BindIntVariable("key_menu_down",      &key_menu_down);
    M_BindIntVariable("key_menu_left",      &key_menu_left);
    M_BindIntVariable("key_menu_right",     &key_menu_right);
    M_BindIntVariable("key_menu_back",      &key_menu_back);
    M_BindIntVariable("key_menu_forward",   &key_menu_forward);
    M_BindIntVariable("key_menu_confirm",   &key_menu_confirm);
    M_BindIntVariable("key_menu_abort",     &key_menu_abort);
    M_BindIntVariable("key_demo_quit",      &key_demo_quit);
    M_BindIntVariable("key_spy",            &key_spy);
}

void M_BindChatControls(unsigned int num_players)
{
    char name[32];  // haleyjd: 20 not large enough - Thank you, come again!
    unsigned int i; // haleyjd: signedness conflict

    M_BindIntVariable("key_multi_msg",     &key_multi_msg);

    for (i=0; i<num_players; ++i)
    {
        M_snprintf(name, sizeof(name), "key_multi_msgplayer%i", i + 1);
        M_BindIntVariable(name, &key_multi_msgplayer[i]);
    }
}

//
// Apply custom patches to the default values depending on the
// platform we are running on.
//

void M_ApplyPlatformDefaults(void)
{
    // no-op. Add your platform-specific patches here.
}

