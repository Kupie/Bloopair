/*
 *   Copyright (C) 2026
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 */
#pragma once

#include <stdint.h>

#ifndef PACKED
#define PACKED __attribute__ ((__packed__))
#endif

#ifndef CHECK_SIZE
#ifdef __cplusplus
#define CHECK_SIZE(type, size) static_assert(sizeof(type) == size, #type " must be " #size " bytes")
#else
#define CHECK_SIZE(type, size) _Static_assert(sizeof(type) == size, #type " must be " #size " bytes")
#endif
#endif

#define WIIMOTE_EXTENSION_NONE 0
#define WIIMOTE_EXTENSION_NUNCHUK 1

typedef struct PACKED {
	// WIIMOTE_EXTENSION_NONE, WIIMOTE_EXTENSION_NUNCHUK (stub for future)
	uint8_t extensionMode;
	// Button that toggles IR pointer on/off. Defaults to BLOOPAIR_PRO_BUTTON_STICK_R
	uint8_t irToggleButton;
	// IR pointer velocity scale (linear), range 1-100, default 50
	uint8_t irVelocity;
} WiimoteConfiguration;
CHECK_SIZE(WiimoteConfiguration, 3);
