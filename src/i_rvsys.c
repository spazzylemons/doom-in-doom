#include "i_rvsys.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <sys/time.h>


//
// TYPES
//
typedef struct
{
    // Should be "IWAD" or "PWAD".
    char		identification[4];		
    int			numlumps;
    int			infotableofs;
    
} mywadinfo_t;


typedef struct
{
    int			filepos;
    int			size;
    char		name[8];
    
} myfilelump_t;

#define NUMSFX 109

static uint32_t current_palette[256];

static SDL_Window *window;
static SDL_Surface *window_surface;
static SDL_Surface *draw_surface;

static Mix_Chunk *sfx_data[NUMSFX];

static mywadinfo_t wadheader;

static int *offsets;
static int *sizes;

static FILE *wadfile;

static char save_name[32];
static FILE *save_file;

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define KEY_RIGHTARROW  0xae
#define KEY_LEFTARROW   0xac
#define KEY_UPARROW     0xad
#define KEY_DOWNARROW   0xaf
#define KEY_ESCAPE      27
#define KEY_ENTER       13
#define KEY_TAB         9
#define KEY_F1          (0x80+0x3b)
#define KEY_F2          (0x80+0x3c)
#define KEY_F3          (0x80+0x3d)
#define KEY_F4          (0x80+0x3e)
#define KEY_F5          (0x80+0x3f)
#define KEY_F6          (0x80+0x40)
#define KEY_F7          (0x80+0x41)
#define KEY_F8          (0x80+0x42)
#define KEY_F9          (0x80+0x43)
#define KEY_F10         (0x80+0x44)
#define KEY_F11         (0x80+0x57)
#define KEY_F12         (0x80+0x58)

#define KEY_BACKSPACE   127
#define KEY_PAUSE       0xff

#define KEY_EQUALS      0x3d
#define KEY_MINUS       0x2d

#define KEY_RSHIFT      (0x80+0x36)
#define KEY_RCTRL       (0x80+0x1d)
#define KEY_RALT        (0x80+0x38)

#define KEY_LALT        KEY_RALT


static int xlatekey(SDL_Keycode sym)
{

    int rc;

    switch(rc = sym)
    {
      case SDLK_LEFT:	rc = KEY_LEFTARROW;	break;
      case SDLK_RIGHT:	rc = KEY_RIGHTARROW;	break;
      case SDLK_DOWN:	rc = KEY_DOWNARROW;	break;
      case SDLK_UP:	rc = KEY_UPARROW;	break;
      case SDLK_ESCAPE:	rc = KEY_ESCAPE;	break;
      case SDLK_RETURN:	rc = KEY_ENTER;		break;
      case SDLK_TAB:	rc = KEY_TAB;		break;
      case SDLK_F1:	rc = KEY_F1;		break;
      case SDLK_F2:	rc = KEY_F2;		break;
      case SDLK_F3:	rc = KEY_F3;		break;
      case SDLK_F4:	rc = KEY_F4;		break;
      case SDLK_F5:	rc = KEY_F5;		break;
      case SDLK_F6:	rc = KEY_F6;		break;
      case SDLK_F7:	rc = KEY_F7;		break;
      case SDLK_F8:	rc = KEY_F8;		break;
      case SDLK_F9:	rc = KEY_F9;		break;
      case SDLK_F10:	rc = KEY_F10;		break;
      case SDLK_F11:	rc = KEY_F11;		break;
      case SDLK_F12:	rc = KEY_F12;		break;
	
      case SDLK_BACKSPACE:
      case SDLK_DELETE:	rc = KEY_BACKSPACE;	break;

      case SDLK_PAUSE:	rc = KEY_PAUSE;		break;

      case SDLK_KP_EQUALS:
      case SDLK_EQUALS:	rc = KEY_EQUALS;	break;

      case SDLK_KP_MINUS:
      case SDLK_MINUS:	rc = KEY_MINUS;		break;

      case SDLK_LSHIFT:
      case SDLK_RSHIFT:
	rc = KEY_RSHIFT;
	break;
	
      case SDLK_LCTRL:
      case SDLK_RCTRL:
	rc = KEY_RCTRL;
	break;
	
      case SDLK_LALT:
      case SDLK_LGUI:
      case SDLK_RALT:
      case SDLK_RGUI:
	rc = KEY_RALT;
	break;
	
      default:
        break;
    }

    return rc;

}

void I_RV_EndFrame(const uint8_t *screen) {
    if (!SDL_LockSurface(draw_surface)) {
        const uint8_t *pixel = screen;
        uint32_t *dest = draw_surface->pixels;
        for (int y = 0; y < 200; y++) {
            for (int x = 0; x < 320; x++) {
                dest[x] = current_palette[*pixel++];
            }
            dest += draw_surface->pitch >> 2;
        }
        SDL_UnlockSurface(draw_surface);
    }
    SDL_BlitSurface(draw_surface, NULL, window_surface, NULL);
    SDL_UpdateWindowSurface(window);

    // temp
    SDL_Delay(1000 / 35);
}

void I_RV_SetPalette(const uint8_t *palette) {
    for (int i = 0; i < 256; i++) {
        uint32_t r = *palette++;
        uint32_t g = *palette++;
        uint32_t b = *palette++;
        current_palette[i] = (r << 24) | (g << 16) | (b << 8) | 0xff;
    }
}

int I_RV_NextEvent(event_t *ev) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                ev->type = ev_keydown;
                ev->data3 = ev->data1 = xlatekey(event.key.keysym.sym);
                return 1;
            case SDL_KEYUP:
                ev->type = ev_keyup;
                ev->data1 = xlatekey(event.key.keysym.sym);
                return 1;
            // Mouse buttons unimplemented yet. Might be in Zscript.
            case SDL_MOUSEMOTION:
                ev->type = ev_mouse;
                ev->data1 = 0;
                ev->data2 = event.motion.xrel;
                ev->data3 = event.motion.yrel;
                return 1;
        }
    }

    return 0;
}

int I_RV_GetNumLumps(void) {
    return wadheader.numlumps;
}

void I_RV_DumpLumps(lumpinfo_t *lumps) {
    fseek(wadfile, wadheader.infotableofs, SEEK_SET);
    offsets = malloc(wadheader.numlumps * sizeof(int));
    sizes = malloc(wadheader.numlumps * sizeof(int));
    for (int i = 0; i < wadheader.numlumps; i++) {
        myfilelump_t fl;
        fread(&fl, sizeof(fl), 1, wadfile);
        memcpy(lumps[i].name, fl.name, 8);
        sizes[i] = lumps[i].size = fl.size;
        offsets[i] = fl.filepos;
    }
}

void I_RV_ReadLump(int lump, void *dest) {
    fseek(wadfile, offsets[lump], SEEK_SET);
    fread(dest, sizes[lump], 1, wadfile);
}

static void get_savename(int id) {
    sprintf(save_name, "save%d.dsg", id);
}

static void savegame_open(int id, const char *mode) {
    get_savename(id);
    save_file = fopen(save_name, mode);
}

int I_RV_SaveRead(void *data, int size) {
    return fread(data, size, 1, save_file);
}

void I_RV_SaveWrite(const void *data, int size) {
    fwrite(data, size, 1, save_file);
}

int I_RV_SaveSize(void) {
    return ftell(save_file);
}

int I_RV_SaveLoad(int id) {
    if (save_file != NULL) {
        fclose(save_file);
    }
    savegame_open(id, "rb");
    return save_file != NULL;
}

void I_RV_SaveStart(int id) {
    if (save_file != NULL) {
        fclose(save_file);
    }
    savegame_open(id, "wb");
}

void I_RV_SaveCommit(void) {
    // TODO no temp file handling, so this is the same as close.
    fclose(save_file);
    save_file = NULL;
}

void I_RV_SaveClose(void) {
    fclose(save_file);
    save_file = NULL;
}

void I_RV_Init(void) {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL_Init failed\n");
        exit(1);
    }

    if (!(window = SDL_CreateWindow("doom", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 200, 0))) {
        printf("SDL_CreateWindow failed\n");
        exit(1);
    }

    if (!(window_surface = SDL_GetWindowSurface(window))) {
        printf("SDL_GetWindowSurface failed\n");
        exit(1);
    }

    if (!(draw_surface = SDL_CreateRGBSurfaceWithFormat(0, 320, 200, 32, SDL_PIXELFORMAT_RGBA8888))) {
        printf("SDL_CreateRGBSurfaceWithFormat failed\n");
        exit(1);
    }

    if (Mix_Init(0)) {
        printf("Mix_Init failed\n");
        exit(1);
    }

    if (Mix_OpenAudio(11025, AUDIO_U8, 1, 1024)) {
        printf("Mix_OpenAudio failed\n");
        exit(1);
    }

    // The WAD stuff will eventually be super easy once this is running in GZ.
    // for now, we gotta do it all manually.
    wadfile = fopen("doom.wad", "rb");
    if (!wadfile) {
        printf("File not found\n");
        exit(1);
    }

    fread(&wadheader, sizeof(wadheader), 1, wadfile);
}
