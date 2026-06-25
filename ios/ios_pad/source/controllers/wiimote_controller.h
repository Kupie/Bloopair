/*
 *   Copyright (C) 2026
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 */
#pragma once

#include "../controllers.h"
#include <bloopair/controllers/wiimote_controller.h>

#define WM_BUTTON_TWO    (1 << 0)
#define WM_BUTTON_ONE    (1 << 1)
#define WM_BUTTON_B      (1 << 2)
#define WM_BUTTON_A      (1 << 3)
#define WM_BUTTON_MINUS  (1 << 4)
#define WM_BUTTON_HOME   (1 << 7)
#define WM_BUTTON_LEFT   (1 << 8)
#define WM_BUTTON_RIGHT  (1 << 9)
#define WM_BUTTON_DOWN   (1 << 10)
#define WM_BUTTON_UP     (1 << 11)
#define WM_BUTTON_PLUS   (1 << 12)

typedef struct PACKED {
	uint8_t report_id;
	uint16_t core_buttons;
} WMCoreReport;
CHECK_SIZE(WMCoreReport, 3);

typedef struct PACKED {
	uint8_t report_id;
	uint16_t core_buttons;
	uint8_t accel[3];
	uint8_t ir[12];
} WMCoreAccelIRReport;
CHECK_SIZE(WMCoreAccelIRReport, 18);

typedef struct PACKED {
	uint8_t irActive;
	int16_t ir_x;
	int16_t ir_y;
	uint8_t extensionMode;
	uint8_t irToggleWasPressed;
	ControllerDeinitFn physicalDeinit;
	void* physicalAdditionalData;
} WiimoteData;
CHECK_SIZE(WiimoteData, 15);

void controllerInit_wiimote(Controller* controller);
void controllerSendInput_wiimote(Controller* controller);
void controllerModuleInit_wiimote(void);
