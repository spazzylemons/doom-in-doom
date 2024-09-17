// RISC-V system routines.

#include "d_event.h"
#include "doomtype.h"
#include "w_wad.h"

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

