//
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
//     Definitions for use in networking code.
//

#ifndef NET_DEFS_H
#define NET_DEFS_H 

#include <stdio.h>

#include "doomtype.h"
#include "d_ticcmd.h"
#include "sha1.h"

// Absolute maximum number of "nodes" in the game.  This is different to
// NET_MAXPLAYERS, as there may be observers that are not participating
// (eg. left/right monitors)

#define MAXNETNODES 16

// The maximum number of players, multiplayer/networking.
// This is the maximum supported by the networking code; individual games
// have their own values for MAXPLAYERS that can be smaller.

#define NET_MAXPLAYERS 8

// Maximum length of a player's name.

#define MAXPLAYERNAME 30

// Networking and tick handling related.

#define BACKUPTICS 128

#define net_client_connected 0
#define drone 0

// net_addr_t

// Game settings sent by client to server when initiating game start,
// and received from the server by clients when the game starts.

typedef struct
{
    int ticdup;
    int extratics;
    int deathmatch;
    int episode;
    int nomonsters;
    int fast_monsters;
    int respawn_monsters;
    int map;
    int skill;
    int gameversion;
    int lowres_turn;
    int new_sync;
    int timelimit;
    int loadgame;
    int random;  // [Strife only]

    // These fields are only used by the server when sending a game
    // start message:

    int num_players;
    int consoleplayer;

    // Hexen player classes:

    int player_classes[NET_MAXPLAYERS];

} net_gamesettings_t;

#define NET_TICDIFF_FORWARD      (1 << 0)
#define NET_TICDIFF_SIDE         (1 << 1)
#define NET_TICDIFF_TURN         (1 << 2)
#define NET_TICDIFF_BUTTONS      (1 << 3)
#define NET_TICDIFF_CONSISTANCY  (1 << 4)
#define NET_TICDIFF_CHATCHAR     (1 << 5)
#define NET_TICDIFF_RAVEN        (1 << 6)
#define NET_TICDIFF_STRIFE       (1 << 7)

typedef struct
{
    unsigned int diff;
    ticcmd_t cmd;
} net_ticdiff_t;

// Complete set of ticcmds from all players

typedef struct 
{
    signed int latency;
    unsigned int seq;
    boolean playeringame[NET_MAXPLAYERS];
    net_ticdiff_t cmds[NET_MAXPLAYERS];
} net_full_ticcmd_t;

#endif /* #ifndef NET_DEFS_H */
