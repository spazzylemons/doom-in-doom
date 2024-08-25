CC := riscv32-unknown-elf-gcc
AS := riscv32-unknown-elf-as
CFLAGS := -march=rv32im -fsigned-char -mabi=ilp32 -g -O3 -nostdinc -Ilibc/include -Wall -Wno-format
LDFLAGS := -nostdlib -flto

O := build

OBJS := \
    $(O)/i_rvsys.o \
    $(O)/am_map.o \
    $(O)/deh_ammo.o \
    $(O)/deh_bexstr.o \
    $(O)/deh_cheat.o \
    $(O)/deh_doom.o \
    $(O)/deh_frame.o \
    $(O)/deh_io.o \
    $(O)/deh_main.o \
    $(O)/deh_mapping.o \
    $(O)/deh_misc.o \
    $(O)/deh_ptr.o \
    $(O)/deh_sound.o \
    $(O)/deh_str.o \
    $(O)/deh_text.o \
    $(O)/deh_thing.o \
    $(O)/deh_weapon.o \
    $(O)/d_event.o \
    $(O)/d_items.o \
    $(O)/d_loop.o \
    $(O)/d_main.o \
    $(O)/d_mode.o \
    $(O)/d_net.o \
    $(O)/doomdef.o \
    $(O)/doomstat.o \
    $(O)/dstrings.o \
    $(O)/f_finale.o \
    $(O)/f_wipe.o \
    $(O)/g_game.o \
    $(O)/hu_lib.o \
    $(O)/hu_stuff.o \
    $(O)/i_main.o \
    $(O)/info.o \
    $(O)/i_sound.o \
    $(O)/i_system.o \
    $(O)/i_timer.o \
    $(O)/i_video.o \
    $(O)/m_argv.o \
    $(O)/m_bbox.o \
    $(O)/m_cheat.o \
    $(O)/m_config.o \
    $(O)/m_controls.o \
    $(O)/memio.o \
    $(O)/m_fixed.o \
    $(O)/m_menu.o \
    $(O)/m_misc.o \
    $(O)/m_random.o \
    $(O)/p_ceilng.o \
    $(O)/p_doors.o \
    $(O)/p_enemy.o \
    $(O)/p_floor.o \
    $(O)/p_inter.o \
    $(O)/p_lights.o \
    $(O)/p_map.o \
    $(O)/p_maputl.o \
    $(O)/p_mobj.o \
    $(O)/p_plats.o \
    $(O)/p_pspr.o \
    $(O)/p_saveg.o \
    $(O)/p_setup.o \
    $(O)/p_sight.o \
    $(O)/p_spec.o \
    $(O)/p_switch.o \
    $(O)/p_telept.o \
    $(O)/p_tick.o \
    $(O)/p_user.o \
    $(O)/r_bsp.o \
    $(O)/r_data.o \
    $(O)/r_draw.o \
    $(O)/r_main.o \
    $(O)/r_plane.o \
    $(O)/r_segs.o \
    $(O)/r_sky.o \
    $(O)/r_things.o \
    $(O)/sha1.o \
    $(O)/sounds.o \
    $(O)/s_sound.o \
    $(O)/st_lib.o \
    $(O)/st_stuff.o \
    $(O)/tables.o \
    $(O)/v_diskicon.o \
    $(O)/v_video.o \
    $(O)/wi_stuff.o \
    $(O)/w_wad.o \
    $(O)/z_zone.o \
    $(O)/ctype.o \
    $(O)/printf.o \
    $(O)/stdio.o \
    $(O)/stdlib.o \
    $(O)/string.o \
    $(O)/crt0.o \

all: doom

clean:
	rm -f $(O)/*

doom: doom.elf
	riscv32-unknown-elf-objcopy $^ -O binary $@

doom.elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -Wl,-Tlink.ld

$(O)/%.o: src/%.c $(wildcard src/*.h) $(wildcard libc/include/*.h)
	$(CC) $(CFLAGS) -flto -c $< -o $@

$(O)/%.o: src/%.S
	$(AS) -march=rv32im -mabi=ilp32 -c $< -o $@

$(O)/%.o: libc/src/%.c $(wildcard src/*.h) $(wildcard libc/include/*.h)
	$(CC) $(CFLAGS) -ffreestanding -c $< -o $@

