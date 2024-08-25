#include "i_rvsys.h"

void I_RV_EndFrame(const uint8_t *screen) {
    register int a0 asm("a0") = 0;
    register int a1 asm("a1") = (uintptr_t) screen;

    asm volatile ( "ecall" :: "r"(a0), "r"(a1) );
}

void I_RV_SetPalette(const uint8_t *palette) {
    register int a0 asm("a0") = 1;
    register int a1 asm("a1") = (uintptr_t) palette;

    asm volatile ( "ecall" :: "r"(a0), "r"(a1) );
}

int I_RV_NextEvent(event_t *ev) {
    register int a0 asm("a0") = 2;
    register int a1 asm("a1") = (uintptr_t) ev;

    asm volatile ( "ecall" : "=r"(a0) : "r"(a0), "r"(a1) );

    return a0;
}

int I_RV_GetNumLumps(void) {
    register int a0 asm("a0") = 3;

    asm volatile ( "ecall" : "=r"(a0) : "r"(a0) );

    return a0;
}

void I_RV_DumpLumps(lumpinfo_t *lumps) {
    register int a0 asm("a0") = 4;
    register int a1 asm("a1") = (uintptr_t) lumps;

    asm volatile ( "ecall" :: "r"(a0), "r"(a1) );
}

void I_RV_ReadLump(int lump, void *dest) {
    register int a0 asm("a0") = 5;
    register int a1 asm("a1") = lump;
    register int a2 asm("a2") = (uintptr_t) dest;

    asm volatile ( "ecall" :: "r"(a0), "r"(a1), "r"(a2) );
}

int I_RV_SaveRead(void *data, int size) {
    register int a0 asm("a0") = 6;
    register int a1 asm("a1") = (uintptr_t) data;
    register int a2 asm("a2") = size;

    asm volatile ( "ecall" : "=r"(a0) : "r"(a0), "r"(a1), "r"(a2) );

    return a0;
}

void I_RV_SaveWrite(const void *data, int size) {
    register int a0 asm("a0") = 7;
    register int a1 asm("a1") = (uintptr_t) data;
    register int a2 asm("a2") = size;

    asm volatile ( "ecall" :: "r"(a0), "r"(a1), "r"(a2) );
}

int I_RV_SaveSize(void) {
    register int a0 asm("a0") = 8;

    asm volatile ( "ecall" : "=r"(a0) : "r"(a0) );

    return a0;
}

int I_RV_SaveLoad(int id) {
    register int a0 asm("a0") = 9;
    register int a1 asm("a1") = id;

    asm volatile ( "ecall" : "=r"(a0) : "r"(a0), "r"(a1) );

    return a0;
}

void I_RV_SaveStart(int id) {
    register int a0 asm("a0") = 10;

    asm volatile ( "ecall" :: "r"(a0) );
}

void I_RV_SaveCommit(void) {
    register int a0 asm("a0") = 11;

    asm volatile ( "ecall" :: "r"(a0) );
}

void I_RV_SaveClose(void) {
    register int a0 asm("a0") = 12;

    asm volatile ( "ecall" :: "r"(a0) );
}

_Noreturn void I_RV_Quit(void) {
    register int a0 asm("a0") = 13;

    asm volatile ( "ecall" :: "r"(a0) );

    __builtin_unreachable();
}

void I_RV_SetMusic(const char *name, int looping) {
    register int a0 asm("a0") = 14;
    register int a1 asm("a1") = (uintptr_t) name;
    register int a2 asm("a2") = looping;

    asm volatile ( "ecall" :: "r"(a0), "r"(a1), "r"(a2) );
}

void I_RV_PlaySound(int channel, int id, int vol, int sep) {
    register int a0 asm("a0") = 15;
    register int a1 asm("a1") = channel;
    register int a2 asm("a2") = id;
    register int a3 asm("a3") = vol;
    register int a4 asm("a4") = sep;

    asm volatile ( "ecall" :: "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4) );
}

void I_RV_UpdateSound(int channel, int vol, int sep) {
    register int a0 asm("a0") = 16;
    register int a1 asm("a1") = channel;
    register int a2 asm("a2") = vol;
    register int a3 asm("a3") = sep;

    asm volatile ( "ecall" :: "r"(a0), "r"(a1), "r"(a2), "r"(a3) );
}

void I_RV_StopSound(int channel) {
    register int a0 asm("a0") = 17;
    register int a1 asm("a1") = channel;

    asm volatile ( "ecall" :: "r"(a0), "r"(a1) );
}

int I_RV_SoundIsPlaying(int channel) {
    register int a0 asm("a0") = 18;
    register int a1 asm("a1") = channel;

    asm volatile ( "ecall" : "=r"(a0) : "r"(a0), "r"(a1) );

    return a0;
}

void _putchar(char c) {
    register int a0 asm("a0") = 19;
    register int a1 asm("a1") = c;

    asm volatile ( "ecall" :: "r"(a0), "r"(a1) );
}

