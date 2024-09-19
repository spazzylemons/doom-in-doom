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

enum evtype_t {
    ev_keydown,
    ev_keyup,
    ev_mouse,
    ev_buttondown,
    ev_buttonup,
    ev_quit
};

enum ccmd_t {
    CCMD_ATTACK,
    CCMD_USE,
    CCMD_FORWARD,
    CCMD_BACK,
    CCMD_MOVELEFT,
    CCMD_MOVERIGHT,
    CCMD_LEFT,
    CCMD_RIGHT,
    CCMD_SPEED,
    CCMD_STRAFE,

    CCMD_WEAPNEXT,
    CCMD_WEAPPREV,

    CCMD_SLOT_1,
    CCMD_SLOT_2,
    CCMD_SLOT_3,
    CCMD_SLOT_4,
    CCMD_SLOT_5,
    CCMD_SLOT_6,
    CCMD_SLOT_7,
    CCMD_SLOT_8,

    CCMD_TOGGLEMAP,
    CCMD_AM_PANLEFT,
    CCMD_AM_PANRIGHT,
    CCMD_AM_PANUP,
    CCMD_AM_PANDOWN,
    CCMD_AM_ZOOMIN,
    CCMD_AM_ZOOMOUT,
    CCMD_AM_GOBIG,
    CCMD_AM_TOGGLEFOLLOW,
    CCMD_AM_TOGGLEGRID,
    CCMD_AM_SETMARK,
    CCMD_AM_CLEARMARKS,

    CCMD_SIZEUP,
    CCMD_SIZEDOWN,

    CCMD_MENU_MAIN,
    CCMD_MENU_HELP,
    CCMD_MENU_SAVE,
    CCMD_MENU_LOAD,
    CCMD_MENU_OPTIONS,
    CCMD_QUICKSAVE,
    CCMD_ENDGAME,
    CCMD_TOGGLEMESSAGES,
    CCMD_QUICKLOAD,
    CCMD_MENU_QUIT,
    CCMD_BUMPGAMMA
};

class event_t {
    evtype_t type;
    int data1, data2, data3;
}
