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
//
//    


#ifndef __D_EVENT__
#define __D_EVENT__


#include "doomtype.h"


//
// Event handling.
//

// Input event types.
typedef enum
{
    // Key press/release events.
    //    data1: Key code (from doomkeys.h) of the key that was
    //           pressed or released. This is the key as it appears
    //           on a US keyboard layout, and does not change with
    //           layout.
    // For ev_keydown only:
    //    data2: ASCII representation of the key that was pressed that
    //           changes with the keyboard layout; eg. if 'Z' is
    //           pressed on a German keyboard, data1='y',data2='z'.
    //           Not affected by modifier keys.
    //    data3: ASCII input, fully modified according to keyboard
    //           layout and any modifier keys that are held down.
    //           Only set if I_StartTextInput() has been called.
    ev_keydown,
    ev_keyup,

    // Mouse movement event.
    //    data1: Bitfield of buttons currently held down.
    //           (bit 0 = left; bit 1 = right; bit 2 = middle).
    //    data2: X axis mouse movement (turn).
    //    data3: Y axis mouse movement (forward/backward).
    ev_mouse,

    // ZScript input events.
    //    data1: Input pressed.
    ev_buttondown,
    ev_buttonup,

    // Quit event. Triggered when the user clicks the "close" button
    // to terminate the application.
    ev_quit
} evtype_t;

// Event structure.
typedef struct
{
    evtype_t type;

    // Event-specific data; see the descriptions given above.
    int data1, data2, data3;
} event_t;

 
//
// Button/action code definitions.
//
typedef enum
{
    // Press "Fire".
    BT_ATTACK		= 1,
    // Use button, to open doors, activate switches.
    BT_USE		= 2,

    // Flag: game events, not really buttons.
    BT_SPECIAL		= 128,
    BT_SPECIALMASK	= 3,
    
    // Flag, weapon change pending.
    // If true, the next 3 bits hold weapon num.
    BT_CHANGE		= 4,
    // The 3bit weapon mask and shift, convenience.
    BT_WEAPONMASK	= (8+16+32),
    BT_WEAPONSHIFT	= 3,

    // Pause the game.
    BTS_PAUSE		= 1,
    // Save the game at each console.
    BTS_SAVEGAME	= 2,

    // Savegame slot numbers
    //  occupy the second byte of buttons.    
    BTS_SAVEMASK	= (4+8+16),
    BTS_SAVESHIFT 	= 2,
  
} buttoncode_t;

// villsa [STRIFE] Strife specific buttons
// TODO - not finished
typedef enum
{
    // Player view look up
    BT2_LOOKUP          = 1,
    // Player view look down
    BT2_LOOKDOWN        = 2,
    // Center player's view
    BT2_CENTERVIEW      = 4,
    // Use inventory item
    BT2_INVUSE          = 8,
    // Drop inventory item
    BT2_INVDROP         = 16,
    // Jump up and down
    BT2_JUMP            = 32,
    // Use medkit
    BT2_HEALTH          = 128,
  
} buttoncode2_t;

// ZScript buttons to handle.
typedef enum
{
    CCMD_ATTACK,
    CCMD_USE,
    CCMD_FORWARD,
    CCMD_BACK,
    CCMD_MOVELEFT,
    CCMD_MOVERIGHT,
    CCMD_LEFT,
    CCMD_RIGHT,
    CCMD_SPEED,
    CCMD_STRAFE,

    CCMD_WEAPNEXT,
    CCMD_WEAPPREV,

    CCMD_SLOT_1,
    CCMD_SLOT_2,
    CCMD_SLOT_3,
    CCMD_SLOT_4,
    CCMD_SLOT_5,
    CCMD_SLOT_6,
    CCMD_SLOT_7,
    CCMD_SLOT_8,

    CCMD_TOGGLEMAP,
    CCMD_AM_PANLEFT,
    CCMD_AM_PANRIGHT,
    CCMD_AM_PANUP,
    CCMD_AM_PANDOWN,
    CCMD_AM_ZOOMIN,
    CCMD_AM_ZOOMOUT,
    CCMD_AM_GOBIG,
    CCMD_AM_TOGGLEFOLLOW,
    CCMD_AM_TOGGLEGRID,
    CCMD_AM_SETMARK,
    CCMD_AM_CLEARMARKS,

    CCMD_SIZEUP,
    CCMD_SIZEDOWN,

    CCMD_MENU_MAIN,
    CCMD_MENU_HELP,
    CCMD_MENU_SAVE,
    CCMD_MENU_LOAD,
    CCMD_MENU_OPTIONS,
    CCMD_QUICKSAVE,
    CCMD_ENDGAME,
    CCMD_TOGGLEMESSAGES,
    CCMD_QUICKLOAD,
    CCMD_MENU_QUIT,
    CCMD_BUMPGAMMA,

    CCMD_COUNT,
} ccmd_t;


// Called by IO functions when input is detected.
void D_PostEvent (event_t *ev);

// Read an event from the event queue

event_t *D_PopEvent(void);


#endif

