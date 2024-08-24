CC := gcc
CFLAGS := -I/usr/include/SDL2
LDFLAGS := -lSDL2 -lSDL2_mixer -lfluidsynth -lsamplerate

O := build

OBJS := \
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
    $(O)/d_iwad.o \
    $(O)/d_loop.o \
    $(O)/d_main.o \
    $(O)/d_mode.o \
    $(O)/d_net.o \
    $(O)/doomdef.o \
    $(O)/doom_icon.o \
    $(O)/doomstat.o \
    $(O)/dstrings.o \
    $(O)/f_finale.o \
    $(O)/f_wipe.o \
    $(O)/g_game.o \
    $(O)/gusconf.o \
    $(O)/hu_lib.o \
    $(O)/hu_stuff.o \
    $(O)/i_cdmus.o \
    $(O)/i_flmusic.o \
    $(O)/i_glob.o \
    $(O)/i_input.o \
    $(O)/i_joystick.o \
    $(O)/i_main.o \
    $(O)/i_musicpack.o \
    $(O)/info.o \
    $(O)/i_sdlmusic.o \
    $(O)/i_sdlsound.o \
    $(O)/i_sound.o \
    $(O)/i_system.o \
    $(O)/i_timer.o \
    $(O)/i_video.o \
    $(O)/i_videohr.o \
    $(O)/m_argv.o \
    $(O)/m_bbox.o \
    $(O)/m_cheat.o \
    $(O)/m_config.o \
    $(O)/m_controls.o \
    $(O)/memio.o \
    $(O)/m_fixed.o \
    $(O)/midifallback.o \
    $(O)/midifile.o \
    $(O)/m_menu.o \
    $(O)/m_misc.o \
    $(O)/m_random.o \
    $(O)/mus2mid.o \
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
    $(O)/statdump.o \
    $(O)/st_lib.o \
    $(O)/st_stuff.o \
    $(O)/tables.o \
    $(O)/v_diskicon.o \
    $(O)/v_video.o \
    $(O)/w_checksum.o \
    $(O)/w_file.o \
    $(O)/w_file_stdc.o \
    $(O)/wi_stuff.o \
    $(O)/w_main.o \
    $(O)/w_merge.o \
    $(O)/w_wad.o \
    $(O)/z_zone.o \

all: doom

clean:
	rm -f $(O)/*

doom: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)

$(O)/%.o: src/%.c $(wildcard src/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

