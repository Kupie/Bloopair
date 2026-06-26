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

#include "wiimote_controller.h"
#include "utils.h"

// Matches WPAD_PRO_AXIS_NORMALIZE_VALUE in controllers.c, the maximum
// magnitude of a stick axis after mapControllerInput() has run.
#define PRO_AXIS_NORMALIZE_VALUE 1140

// Emulated IR camera resolution, see <https://wiibrew.org/wiki/Wiimote#IR_Camera>
#define IR_CENTER_X 512
#define IR_CENTER_Y 384

static const WiimoteConfiguration default_wiimote_configuration = {
    .extensionMode = WIIMOTE_EXTENSION_NONE,
    .irToggleButton = BLOOPAIR_PRO_BUTTON_STICK_R,
    .irVelocity = 50,
};

static const MappingConfiguration default_wiimote_mapping = {
    .num = 13,
    .mappings = {
        { BLOOPAIR_PRO_BUTTON_UP,      WIIMOTE_BUTTON_UP, },
        { BLOOPAIR_PRO_BUTTON_DOWN,    WIIMOTE_BUTTON_DOWN, },
        { BLOOPAIR_PRO_BUTTON_LEFT,    WIIMOTE_BUTTON_LEFT, },
        { BLOOPAIR_PRO_BUTTON_RIGHT,   WIIMOTE_BUTTON_RIGHT, },

        { BLOOPAIR_PRO_BUTTON_A,       WIIMOTE_BUTTON_TWO, },
        { BLOOPAIR_PRO_BUTTON_B,       WIIMOTE_BUTTON_ONE, },
        { BLOOPAIR_PRO_BUTTON_X,       WIIMOTE_BUTTON_A, },
        { BLOOPAIR_PRO_BUTTON_Y,       WIIMOTE_BUTTON_B, },

        { BLOOPAIR_PRO_TRIGGER_L,      WIIMOTE_BUTTON_MINUS, },
        { BLOOPAIR_PRO_TRIGGER_ZL,     WIIMOTE_BUTTON_MINUS, },
        { BLOOPAIR_PRO_TRIGGER_R,      WIIMOTE_BUTTON_PLUS, },
        { BLOOPAIR_PRO_TRIGGER_ZR,     WIIMOTE_BUTTON_PLUS, },

        { BLOOPAIR_PRO_BUTTON_HOME,    WIIMOTE_BUTTON_HOME, },
    },
};

// Maps the already-mapped (stage 1) Wii U Pro Controller buttons to the
// emulated Wiimote's core button bitfield. This is a separate, much simpler
// loop than mapControllerInput() since Wiimote buttons aren't a bitfield
// the generic stick/button mapping logic understands.
static uint16_t mapProButtonsToCore(MappingConfiguration* mapping, uint32_t proButtons)
{
    uint16_t core_buttons = 0;

    for (uint8_t i = 0; i < mapping->num; i++) {
        const BloopairMappingEntry* e = &mapping->mappings[i];
        if (proButtons & BTN(e->from)) {
            core_buttons |= (1 << e->to);
        }
    }

    return core_buttons;
}

// Encodes a single IR dot pair (2 of the camera's 4 tracked dots) into the
// 5-byte wire format used by 0x33 reports. We only ever emit one dot, the
// second slot of the pair is always reported as not found (all bits set).
static void encodeIRDotPair(WMIRDotPair* out, uint8_t active, uint16_t x, uint16_t y)
{
    if (!active) {
        memset(out, 0xff, sizeof(*out));
        return;
    }

    out->x = x & 0xff;
    out->y = y & 0xff;
    out->x2_y2_xhi_yhi = ((x >> 8) & 0x3) << 6 | ((y >> 8) & 0x3) << 4 | 0xf;
    out->x3 = 0xff;
    out->y3 = 0xff;
}

void controllerDeinit_wiimote(Controller* controller)
{
    WiimoteData* wdata = (WiimoteData*) controller->wiimoteData;

    // call the underlying hardware module's own deinit first so it can free
    // its own additionalData, then free our own state
    if (wdata->hwDeinit) {
        wdata->hwDeinit(controller);
    }

    IOS_Free(LOCAL_PROCESS_HEAP_ID, wdata);
    controller->wiimoteData = NULL;
}

void controllerSendInput_wiimote(Controller* controller)
{
    WiimoteData* wdata = (WiimoteData*) controller->wiimoteData;

    // stage 1: map raw hardware buttons to Wii U Pro Controller buttons,
    // exactly like the Pro Controller path does. controller->mapping and
    // mapControllerInput() are completely untouched by Wiimote emulation.
    BloopairReportBuffer repBuf;
    mapControllerInput(controller, &controller->reportBuffer, &repBuf);

    // stage 2: map Pro Controller buttons to the emulated Wiimote's buttons
    uint16_t core_buttons = mapProButtonsToCore(wdata->mapping, repBuf.buttons);

    // toggle the emulated IR pointer on a rising edge of the configured button
    uint8_t irTogglePressed = (repBuf.buttons & BTN(wdata->config->irToggleButton)) != 0;
    if (irTogglePressed && !wdata->irToggleWasPressed) {
        wdata->irActive = !wdata->irActive;
    }
    wdata->irToggleWasPressed = irTogglePressed;

    if (wdata->irActive) {
        // drive the IR pointer with the right stick, scaled by irVelocity
        int32_t ir_x = IR_CENTER_X + (repBuf.right_stick_x * wdata->config->irVelocity) / PRO_AXIS_NORMALIZE_VALUE;
        int32_t ir_y = IR_CENTER_Y - (repBuf.right_stick_y * wdata->config->irVelocity) / PRO_AXIS_NORMALIZE_VALUE;
        ir_x = CLAMP(ir_x, 0, 1023);
        ir_y = CLAMP(ir_y, 0, 1023);

        WMCoreAccelIRReport report;
        report.report_id = WM_CORE_ACCEL_IR_REPORT_ID;
        report.core_buttons = core_buttons;
        // accelerometer data is not implemented, report the resting position
        memset(report.accel, 0, sizeof(report.accel));
        encodeIRDotPair(&report.ir[0], 1, ir_x, ir_y);
        encodeIRDotPair(&report.ir[1], 0, 0, 0);

        sendInputData(controller->handle, &report, sizeof(report));
    } else {
        WMCoreReport report;
        report.report_id = WM_CORE_REPORT_ID;
        report.core_buttons = core_buttons;

        sendInputData(controller->handle, &report, sizeof(report));
    }
}

void Controller_ApplyWiimoteModeIfEnabled(Controller* controller)
{
    if (!Configuration_GetWiimoteMode(controller->type, controller->bda)) {
        return;
    }

    WiimoteData* wdata = (WiimoteData*) IOS_Alloc(LOCAL_PROCESS_HEAP_ID, sizeof(WiimoteData));
    if (!wdata) {
        return;
    }
    memset(wdata, 0, sizeof(WiimoteData));

    BloopairCommonConfiguration* common = NULL;
    void* custom = NULL;
    uint32_t customSize = 0;
    if (Configuration_GetAll(BLOOPAIR_CONTROLLER_WIIMOTE, NULL,
            &common, &wdata->mapping, &custom, &customSize) < 0) {
        IOS_Free(LOCAL_PROCESS_HEAP_ID, wdata);
        return;
    }

    // we don't actually use the common configuration for Wiimote emulation,
    // the relevant settings live in the custom configuration instead
    wdata->config = (custom && customSize >= sizeof(WiimoteConfiguration))
        ? (WiimoteConfiguration*) custom
        : (WiimoteConfiguration*) &default_wiimote_configuration;

    // overlay our own deinit/data callbacks, saving the hardware module's
    // deinit so it still runs and frees its own additionalData
    wdata->hwDeinit = controller->deinit;
    controller->deinit = controllerDeinit_wiimote;
    controller->wiimoteData = wdata;

    controller->type = BLOOPAIR_CONTROLLER_WIIMOTE;
}

void controllerModuleInit_wiimote(void)
{
    Configuration_SetFallback(BLOOPAIR_CONTROLLER_WIIMOTE, NULL, &default_wiimote_mapping,
        &default_wiimote_configuration, sizeof(default_wiimote_configuration));
}
