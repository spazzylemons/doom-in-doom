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
//     Main loop code.
//

#include <stdlib.h>
#include <string.h>

#include "d_event.h"
#include "d_loop.h"
#include "d_ticcmd.h"

#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"

#include "m_argv.h"
#include "m_fixed.h"

#include "net_defs.h"

// The complete set of data for a particular tic.

typedef struct
{
    ticcmd_t cmds[NET_MAXPLAYERS];
    boolean ingame[NET_MAXPLAYERS];
} ticcmd_set_t;

// Maximum time that we wait in TryRunTics() for netgame data to be
// received before we bail out and render a frame anyway.
// Vanilla Doom used 20 for this value, but we use a smaller value
// instead for better responsiveness of the menu when we're stuck.
#define MAX_NETGAME_STALL_TICS  5

//
// gametic is the tic about to (or currently being) run
// maketic is the tic that hasn't had control made for it yet
// recvtic is the latest tic received from the server.
//
// a gametic cannot be run until ticcmds are received for it
// from all players.
//

static ticcmd_set_t ticdata[BACKUPTICS];

// The index of the next tic to be made (with a call to BuildTiccmd).

static int maketic;

// The number of complete tics received from the server so far.

static int recvtic;

// The number of tics that have been run (using RunTic) so far.

int gametic;

// Index of the local player.

static int localplayer;

// Reduce the bandwidth needed by sampling game input less and transmitting
// less.  If ticdup is 2, sample half normal, 3 = one third normal, etc.

int		ticdup;

// Amount to offset the timer for game sync.

fixed_t         offsetms;

// Use new client syncronisation code

static boolean  new_sync = true;

// Callback functions for loop code.

static loop_interface_t *loop_interface = NULL;

// Current players in the multiplayer game.
// This is distinct from playeringame[] used by the game code, which may
// modify playeringame[] when playing back multiplayer demos.

static boolean local_playeringame[NET_MAXPLAYERS];

// Requested player class "sent" to the server on connect.
// If we are only doing a single player game then this needs to be remembered
// and saved in the game settings.

static int player_class;

static boolean BuildNewTic(void)
{
    int	gameticdiv;
    ticcmd_t cmd;

    gameticdiv = gametic/ticdup;

    I_StartTic ();
    loop_interface->ProcessEvents();

    // Always run the menu

    loop_interface->RunMenu();

    if (new_sync)
    {
       // If playing single player, do not allow tics to buffer
       // up very far

       if (maketic - gameticdiv > 2)
           return false;

       // Never go more than ~200ms ahead

       if (maketic - gameticdiv > 8)
           return false;
    }
    else
    {
       if (maketic - gameticdiv >= 5)
           return false;
    }

    //printf ("mk:%i ",maketic);
    memset(&cmd, 0, sizeof(ticcmd_t));
    loop_interface->BuildTiccmd(&cmd, maketic);

    ticdata[maketic % BACKUPTICS].cmds[localplayer] = cmd;
    ticdata[maketic % BACKUPTICS].ingame[localplayer] = true;

    ++maketic;

    return true;
}

static void D_Disconnected(void)
{
    // disconnected from server

    printf("Disconnected from server.\n");
}

//
// Invoked by the network engine when a complete set of ticcmds is
// available.
//

void D_ReceiveTic(ticcmd_t *ticcmds, boolean *players_mask)
{
    int i;

    // Disconnected from server?

    if (ticcmds == NULL && players_mask == NULL)
    {
        D_Disconnected();
        return;
    }

    for (i = 0; i < NET_MAXPLAYERS; ++i)
    {
        if (i == localplayer)
        {
            // This is us.  Don't overwrite it.
        }
        else
        {
            ticdata[recvtic % BACKUPTICS].cmds[i] = ticcmds[i];
            ticdata[recvtic % BACKUPTICS].ingame[i] = players_mask[i];
        }
    }

    ++recvtic;
}

//
// Start game loop
//
// Called after the screen is set but before the game starts running.
//

void D_StartGameLoop(void)
{
}

void D_StartNetGame(net_gamesettings_t *settings)
{
    int i;

    offsetms = 0;
    recvtic = 0;

    settings->consoleplayer = 0;
    settings->num_players = 1;
    settings->player_classes[0] = player_class;

    //!
    // @category net
    //
    // Use original network client sync code rather than the improved
    // sync code.
    //
    settings->new_sync = !M_ParmExists("-oldsync");

    //!
    // @category net
    // @arg <n>
    //
    // Send n extra tics in every packet as insurance against dropped
    // packets.
    //

    i = M_CheckParmWithArgs("-extratics", 1);

    if (i > 0)
        settings->extratics = atoi(myargv[i+1]);
    else
        settings->extratics = 1;

    //!
    // @category net
    // @arg <n>
    //
    // Reduce the resolution of the game by a factor of n, reducing
    // the amount of network bandwidth needed.
    //

    i = M_CheckParmWithArgs("-dup", 1);

    if (i > 0)
        settings->ticdup = atoi(myargv[i+1]);
    else
        settings->ticdup = 1;

    // Set the local player and playeringame[] values.

    localplayer = settings->consoleplayer;

    for (i = 0; i < NET_MAXPLAYERS; ++i)
    {
        local_playeringame[i] = i < settings->num_players;
    }

    // Copy settings to global variables.

    ticdup = settings->ticdup;
    new_sync = settings->new_sync;

    if (ticdup < 1)
    {
        I_Error("D_StartNetGame: invalid ticdup value (%d)", ticdup);
    }

    // TODO: Message disabled until we fix new_sync.
    //if (!new_sync)
    //{
    //    printf("Syncing netgames like Vanilla Doom.\n");
    //}
}

// When using ticdup, certain values must be cleared out when running
// the duplicate ticcmds.

static void TicdupSquash(ticcmd_set_t *set)
{
    ticcmd_t *cmd;
    unsigned int i;

    for (i = 0; i < NET_MAXPLAYERS ; ++i)
    {
        cmd = &set->cmds[i];
        cmd->chatchar = 0;
        if (cmd->buttons & BT_SPECIAL)
            cmd->buttons = 0;
    }
}

// When running in single player mode, clear all the ingame[] array
// except the local player.

static void SinglePlayerClear(ticcmd_set_t *set)
{
    unsigned int i;

    for (i = 0; i < NET_MAXPLAYERS; ++i)
    {
        if (i != localplayer)
        {
            set->ingame[i] = false;
        }
    }
}

//
// TryRunTics
//

void TryRunTics (void)
{
    int	i;
    int	lowtic;
    int	entertic;
    static int oldentertics;
    int realtics;
    int	availabletics;
    int	counts;

    // get real tics
    entertic = I_GetTime() / ticdup;
    realtics = entertic - oldentertics;
    oldentertics = entertic;

    // Run a single tic every time this function

    BuildNewTic();

    lowtic = maketic;

    availabletics = lowtic - gametic/ticdup;

    // decide how many tics to run

    if (new_sync)
    {
	counts = availabletics;
    }
    else
    {
        // decide how many tics to run
        if (realtics < availabletics-1)
            counts = realtics+1;
        else if (realtics < availabletics)
            counts = realtics;
        else
            counts = availabletics;

        if (counts < 1)
            counts = 1;
    }

    if (counts < 1)
	counts = 1;

    // wait for new tics if needed
    while (lowtic < gametic/ticdup + counts)
    {
        lowtic = maketic;

	if (lowtic < gametic/ticdup)
	    I_Error ("TryRunTics: lowtic < gametic");

        // Still no tics to run? Sleep until some are available.
        if (lowtic < gametic/ticdup + counts)
        {
            I_EndFrame();
            return;
        }
    }

    // run the count * ticdup dics
    while (counts--)
    {
        ticcmd_set_t *set;

        set = &ticdata[(gametic / ticdup) % BACKUPTICS];

        SinglePlayerClear(set);

	for (i=0 ; i<ticdup ; i++)
	{
            if (gametic/ticdup > lowtic)
                I_Error ("gametic>lowtic");

            memcpy(local_playeringame, set->ingame, sizeof(local_playeringame));

            loop_interface->RunTic(set->cmds, set->ingame);
	    gametic++;

	    // modify command for duplicated tics

            TicdupSquash(set);
	}
    }

    I_EndFrame();
}

void D_RegisterLoopCallbacks(loop_interface_t *i)
{
    loop_interface = i;
}

// TODO: Move nonvanilla demo functions into a dedicated file.
#include "m_misc.h"
#include "w_wad.h"

static boolean StrictDemos(void)
{
    //!
    // @category demo
    //
    // When recording or playing back demos, disable any extensions
    // of the vanilla demo format - record demos as vanilla would do,
    // and play back demos as vanilla would do.
    //
    return M_ParmExists("-strictdemos");
}

// If the provided conditional value is true, we're trying to record
// a demo file that will include a non-vanilla extension. The function
// will return true if the conditional is true and it's allowed to use
// this extension (no extensions are allowed if -strictdemos is given
// on the command line). A warning is shown on the console using the
// provided string describing the non-vanilla expansion.
boolean D_NonVanillaRecord(boolean conditional, const char *feature)
{
    if (!conditional || StrictDemos())
    {
        return false;
    }

    printf("Warning: Recording a demo file with a non-vanilla extension "
           "(%s). Use -strictdemos to disable this extension.\n",
           feature);

    return true;
}

// If the provided conditional value is true, we're trying to play back
// a demo that includes a non-vanilla extension. We return true if the
// conditional is true and it's allowed to use this extension, checking
// that:
//  - The -strictdemos command line argument is not provided.
//  - The given lumpnum identifying the demo to play back identifies a
//    demo that comes from a .lmp file, not a .wad file.
//  - Before proceeding, a warning is shown to the user on the console.
boolean D_NonVanillaPlayback(boolean conditional, int lumpnum,
                             const char *feature)
{
    if (!conditional || StrictDemos())
    {
        return false;
    }

    printf("Warning: Playing back a demo file with a non-vanilla extension "
           "(%s). Use -strictdemos to disable this extension.\n",
           feature);

    return true;
}

