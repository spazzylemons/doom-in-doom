CC := clang
AS := llvm-as
LD := llvm-link

SHELL=/bin/bash -eou pipefail

CFLAGS := --target=i386-unknown \
	-Oz \
	-nostdinc \
	-fno-vectorize \
	-fno-slp-vectorize \
	-Ilibc/include \
	-Wall \
	-S \
	-emit-llvm \

O := build

TARGETDIR := wadsrc/DoomInDoom/generated

TARGET := $(TARGETDIR)/code.zs

BITCODE := $(O)/doom.bc

OBJS := \
    $(O)/am_map.bc \
    $(O)/deh_ammo.bc \
    $(O)/deh_bexstr.bc \
    $(O)/deh_cheat.bc \
    $(O)/deh_doom.bc \
    $(O)/deh_frame.bc \
    $(O)/deh_io.bc \
    $(O)/deh_main.bc \
    $(O)/deh_mapping.bc \
    $(O)/deh_misc.bc \
    $(O)/deh_ptr.bc \
    $(O)/deh_sound.bc \
    $(O)/deh_str.bc \
    $(O)/deh_text.bc \
    $(O)/deh_thing.bc \
    $(O)/deh_weapon.bc \
    $(O)/d_event.bc \
    $(O)/d_items.bc \
    $(O)/d_loop.bc \
    $(O)/d_main.bc \
    $(O)/d_mode.bc \
    $(O)/d_net.bc \
    $(O)/doomstat.bc \
    $(O)/dstrings.bc \
    $(O)/f_finale.bc \
    $(O)/f_wipe.bc \
    $(O)/g_game.bc \
    $(O)/hu_lib.bc \
    $(O)/hu_stuff.bc \
    $(O)/info.bc \
    $(O)/i_sound.bc \
    $(O)/i_system.bc \
    $(O)/i_video.bc \
    $(O)/m_argv.bc \
    $(O)/m_bbox.bc \
    $(O)/m_cheat.bc \
    $(O)/m_controls.bc \
    $(O)/memio.bc \
    $(O)/m_menu.bc \
    $(O)/m_misc.bc \
    $(O)/m_random.bc \
    $(O)/p_ceilng.bc \
    $(O)/p_doors.bc \
    $(O)/p_enemy.bc \
    $(O)/p_floor.bc \
    $(O)/p_inter.bc \
    $(O)/p_lights.bc \
    $(O)/p_map.bc \
    $(O)/p_maputl.bc \
    $(O)/p_mobj.bc \
    $(O)/p_plats.bc \
    $(O)/p_pspr.bc \
    $(O)/p_saveg.bc \
    $(O)/p_setup.bc \
    $(O)/p_sight.bc \
    $(O)/p_spec.bc \
    $(O)/p_switch.bc \
    $(O)/p_telept.bc \
    $(O)/p_tick.bc \
    $(O)/p_user.bc \
    $(O)/r_bsp.bc \
    $(O)/r_data.bc \
    $(O)/r_draw.bc \
    $(O)/r_main.bc \
    $(O)/r_plane.bc \
    $(O)/r_segs.bc \
    $(O)/r_sky.bc \
    $(O)/r_things.bc \
    $(O)/sha1.bc \
    $(O)/sounds.bc \
    $(O)/s_sound.bc \
    $(O)/st_lib.bc \
    $(O)/st_stuff.bc \
    $(O)/tables.bc \
    $(O)/v_video.bc \
    $(O)/wi_stuff.bc \
    $(O)/w_wad.bc \
    $(O)/z_zone.bc \
    $(O)/ctype.bc \
    $(O)/printf.bc \
    $(O)/stdio.bc \
    $(O)/stdlib.bc \
    $(O)/string.bc \

CONVERTER := converter/build/converter

.PHONY: all clean

all: $(TARGET)

$(O):
	mkdir -p $(O)

clean:
	rm -rf $(O)
	rm -f $(BITCODE)
	rm -f $(TARGETDIR)/code.zs
	rm -f $(TARGETDIR)/data.bin

$(TARGET): $(BITCODE) $(CONVERTER)
	./$(CONVERTER) $(BITCODE) $(TARGETDIR)

$(CONVERTER): $(wildcard converter/src/*) converter/CMakeLists.txt
	cd converter && cmake -B build && cd build && $(MAKE)

$(BITCODE): $(OBJS)
	$(LD) $^ -o $@

$(O)/%.bc: src/%.c $(O) $(wildcard src/*.h) $(wildcard libc/include/*.h)
	$(CC) $(CFLAGS) -c $< -o /dev/stdout | $(AS) /dev/stdin -o $@

$(O)/%.bc: libc/src/%.c $(O) $(wildcard src/*.h) $(wildcard libc/include/*.h)
	$(CC) $(CFLAGS) -ffreestanding -c $< -o /dev/stdout | $(AS) /dev/stdin -o $@

