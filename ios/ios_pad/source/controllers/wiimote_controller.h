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

#include <controllers.h>
#include <bloopair/controllers/wiimote_controller.h>

// Information about the reports can be found here:
// - <https://wiibrew.org/wiki/Wiimote#Buttons>
// - <https://wiibrew.org/wiki/Wiimote#Basic_Mode>

#define WM_CORE_REPORT_ID           0x30
#define WM_CORE_ACCEL_IR_REPORT_ID  0x33

#define WM_BUTTON_TWO    (1 << WIIMOTE_BUTTON_TWO)
#define WM_BUTTON_ONE    (1 << WIIMOTE_BUTTON_ONE)
#define WM_BUTTON_B      (1 << WIIMOTE_BUTTON_B)
#define WM_BUTTON_A      (1 << WIIMOTE_BUTTON_A)
#define WM_BUTTON_MINUS  (1 << WIIMOTE_BUTTON_MINUS)
#define WM_BUTTON_HOME   (1 << WIIMOTE_BUTTON_HOME)
#define WM_BUTTON_LEFT   (1 << WIIMOTE_BUTTON_LEFT)
#define WM_BUTTON_RIGHT  (1 << WIIMOTE_BUTTON_RIGHT)
#define WM_BUTTON_DOWN   (1 << WIIMOTE_BUTTON_DOWN)
#define WM_BUTTON_UP     (1 << WIIMOTE_BUTTON_UP)
#define WM_BUTTON_PLUS   (1 << WIIMOTE_BUTTON_PLUS)

typedef struct PACKED {
    uint8_t report_id;
    uint16_t core_buttons;
} WMCoreReport;
CHECK_SIZE(WMCoreReport, 3);

// IR object data, 5 bytes per pair of dots. We only ever emit a single dot,
// the second one is always sent as inactive (fully 0xff).
typedef struct PACKED {
    uint8_t x;
    uint8_t y;
    uint8_t x2_y2_xhi_yhi;
    uint8_t x3;
    uint8_t y3;
} WMIRDotPair;
CHECK_SIZE(WMIRDotPair, 5);

typedef struct PACKED {
    uint8_t report_id;
    uint16_t core_buttons;
    uint8_t accel[3];
    WMIRDotPair ir[2];
} WMCoreAccelIRReport;
CHECK_SIZE(WMCoreAccelIRReport, 16);

// Internal, non-wire-format state for an emulated Wiimote.
typedef struct {
    // is the emulated IR pointer currently active
    uint8_t irActive;
    // edge detection for the IR toggle button
    uint8_t irToggleWasPressed;

    // configuration resolved for this controller
    MappingConfiguration* mapping;
    WiimoteConfiguration* config;

    // the underlying hardware module's deinit function, called from
    // controllerDeinit_wiimote() so its own state still gets cleaned up
    ControllerDeinitFn hwDeinit;
} WiimoteData;

// Checks the configuration for the controller's BDA/type and, if Wiimote
// emulation mode is enabled, overlays it on top of the already-initialized
// hardware controller module. Safe to call unconditionally after any
// controllerInit_*() call.
void Controller_ApplyWiimoteModeIfEnabled(Controller* controller);

// Sends the current input state for a controller running in Wiimote
// emulation mode. Counterpart to sendControllerInput() for the Pro
// Controller path, used instead of it when controller->wiimoteData is set.
void controllerSendInput_wiimote(Controller* controller);

void controllerModuleInit_wiimote(void);
