/**
 * DoomInDoom - Doom compiled to ZScript
 * Copyright (C) 2024 spazzylemons
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

version "4.12.2"

#include "DoomInDoom/DoomInDoom.zs"
#include "DoomInDoom/VAList.zs"

#include "DoomInDoom/generated/code.zs"

#include "DoomInDoom/interface/d_event.zs"
#include "DoomInDoom/interface/doomkeys.zs"
#include "DoomInDoom/interface/g_game.zs"
#include "DoomInDoom/interface/i_sound.zs"
#include "DoomInDoom/interface/i_system.zs"
#include "DoomInDoom/interface/i_video.zs"
#include "DoomInDoom/interface/m_fixed.zs"
#include "DoomInDoom/interface/w_wad.zs"

#include "DoomInDoom/HelpScreen.zs"
#include "DoomInDoom/InputListener.zs"
