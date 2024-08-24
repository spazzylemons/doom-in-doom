// RISC-V system routines.

#include "d_event.h"
#include "doomtype.h"
#include "w_wad.h"

// Temporary for local.
void I_RV_Init(void);

// End the current frame. Pass in the screen to draw.
void I_RV_EndFrame(const byte *screen);

// Update the palette.
void I_RV_SetPalette(const byte *palette);

// Get the next event. Returns 1 if event occurred.
int I_RV_NextEvent(event_t *ev);

// Get the number of lumps.
int I_RV_GetNumLumps(void);

// Get lump info.
void I_RV_DumpLumps(lumpinfo_t *lumps);

// Read a lump.
void I_RV_ReadLump(int lump, void *dest);

// Get the size of a save file.
int I_RV_FileSize(int id);

// Load a save file. Size is provided if less bytes want to be read (save game strings)
// Returns 1 if read was successful.
int I_RV_LoadFile(int id, void *data, int size);

// Save a save file.
void I_RV_SaveFile(int id, const void *data, int size);

// Get the number of tics.
int I_RV_GetTime(void);

_Noreturn void I_RV_Quit(void);

