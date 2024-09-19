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

const ERROR_MESSAGE_BUF_SIZE = 512;

extend class DoomInDoom {
    private bool alreadyQuitting;

    void func_I_Error(uint error, VAList v) {
        if (alreadyQuitting) {
            ThrowAbortException("Warning: recursive call to I_Error detected.");
        }
        alreadyQuitting = true;

        uint argptr = Alloca(4, 4);
        uint msgbuf = Alloca(ERROR_MESSAGE_BUF_SIZE, 1);
        v.Start(argptr);
        func_M_vsnprintf(msgbuf, ERROR_MESSAGE_BUF_SIZE, error, Load32(argptr));
        v.End(argptr);

        ThrowAbortException(GetString(msgbuf));
    }

    void func_I_Exit() {
        Die(self, self);
    }
}
