// RISC-V system routines.

#include "d_event.h"
#include "doomtype.h"
#include "w_wad.h"

// Get the next event. Returns 1 if event occurred.
int I_RV_NextEvent(event_t *ev);

// Get the number of lumps.
int I_RV_GetNumLumps(void);

// Get lump info.
void I_RV_DumpLumps(lumpinfo_t *lumps);

// Read a lump.
void I_RV_ReadLump(int lump, void *dest);

// SAVE FILES //

int I_RV_SaveRead(void *data, int size);

void I_RV_SaveWrite(const void *data, int size);

int I_RV_SaveSize(void);

int I_RV_SaveLoad(int id);

void I_RV_SaveStart(int id);

void I_RV_SaveCommit(void);

void I_RV_SaveClose(void);

// MISC //

_Noreturn void I_RV_Quit(void);

