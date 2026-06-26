/*
 *   Copyright (C) 2024 GaryOderNichts
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include "common.h"

enum {
    WIIMOTE_EXTENSION_NONE,
    WIIMOTE_EXTENSION_NUNCHUK,
};

// Bit positions in the emulated Wiimote core buttons field.
// See <https://wiibrew.org/wiki/Wiimote#Buttons>
enum WiimoteButton {
    WIIMOTE_BUTTON_TWO   = 0,
    WIIMOTE_BUTTON_ONE   = 1,
    WIIMOTE_BUTTON_B     = 2,
    WIIMOTE_BUTTON_A     = 3,
    WIIMOTE_BUTTON_MINUS = 4,
    WIIMOTE_BUTTON_HOME  = 7,
    WIIMOTE_BUTTON_LEFT  = 8,
    WIIMOTE_BUTTON_RIGHT = 9,
    WIIMOTE_BUTTON_DOWN  = 10,
    WIIMOTE_BUTTON_UP    = 11,
    WIIMOTE_BUTTON_PLUS  = 12,
};

typedef struct {
    // one of WIIMOTE_EXTENSION_*, only WIIMOTE_EXTENSION_NONE is currently implemented
    uint8_t extensionMode;
    // one of the BloopairProButtons used to toggle IR pointer emulation on/off
    uint8_t irToggleButton;
    // how fast the emulated IR dot moves per report when held off-center
    uint8_t irVelocity;
} WiimoteConfiguration;
