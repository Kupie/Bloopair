/*
 *   Copyright (C) 2026
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 */

#include "wiimote_controller.h"
#include <bloopair/controllers/switch_controller.h>

#define WIIMOTE_IR_CENTER_X 511
#define WIIMOTE_IR_CENTER_Y 383
#define WIIMOTE_IR_MAX_X 1023
#define WIIMOTE_IR_MAX_Y 767
#define WIIMOTE_IR_DOT_DISTANCE 64
#define WIIMOTE_IR_DOT_SIZE 15
#define WIIMOTE_IR_DEADZONE 80
#define WIIMOTE_IR_STEP_DIVISOR 200

static const WiimoteConfiguration default_wiimote_configuration = {
	.extensionMode = WIIMOTE_EXTENSION_NONE,
	.irToggleButton = BLOOPAIR_PRO_BUTTON_STICK_R,
	.irVelocity = 50,
};

static const MappingConfiguration default_wiimote_mapping = {
	.num = 13,
	.mappings = {
		{ SWITCH_BUTTON_UP,              BLOOPAIR_PRO_BUTTON_UP, },
		{ SWITCH_BUTTON_LEFT,            BLOOPAIR_PRO_BUTTON_LEFT, },
		{ SWITCH_BUTTON_DOWN,            BLOOPAIR_PRO_BUTTON_DOWN, },
		{ SWITCH_BUTTON_RIGHT,           BLOOPAIR_PRO_BUTTON_RIGHT, },
		{ SWITCH_BUTTON_A,               BLOOPAIR_PRO_BUTTON_TWO, },
		{ SWITCH_BUTTON_B,               BLOOPAIR_PRO_BUTTON_ONE, },
		{ SWITCH_BUTTON_X,               BLOOPAIR_PRO_BUTTON_A, },
		{ SWITCH_BUTTON_Y,               BLOOPAIR_PRO_BUTTON_B, },
		{ SWITCH_TRIGGER_L,              BLOOPAIR_PRO_BUTTON_MINUS, },
		{ SWITCH_TRIGGER_ZL,             BLOOPAIR_PRO_BUTTON_MINUS, },
		{ SWITCH_TRIGGER_R,              BLOOPAIR_PRO_BUTTON_PLUS, },
		{ SWITCH_TRIGGER_ZR,             BLOOPAIR_PRO_BUTTON_PLUS, },
		{ SWITCH_BUTTON_HOME,            BLOOPAIR_PRO_BUTTON_HOME, },
	},
};

static uint16_t wiimoteButtonsFromMappedButtons(uint32_t buttons)
{
	uint16_t core_buttons = 0;

	if (buttons & BTN(BLOOPAIR_PRO_BUTTON_UP)) core_buttons |= WM_BUTTON_UP;
	if (buttons & BTN(BLOOPAIR_PRO_BUTTON_LEFT)) core_buttons |= WM_BUTTON_LEFT;
	if (buttons & BTN(BLOOPAIR_PRO_BUTTON_DOWN)) core_buttons |= WM_BUTTON_DOWN;
	if (buttons & BTN(BLOOPAIR_PRO_BUTTON_RIGHT)) core_buttons |= WM_BUTTON_RIGHT;
	if (buttons & BTN(BLOOPAIR_PRO_BUTTON_A)) core_buttons |= WM_BUTTON_A;
	if (buttons & BTN(BLOOPAIR_PRO_BUTTON_B)) core_buttons |= WM_BUTTON_B;
	if (buttons & BTN(BLOOPAIR_PRO_BUTTON_MINUS)) core_buttons |= WM_BUTTON_MINUS;
	if (buttons & BTN(BLOOPAIR_PRO_BUTTON_PLUS)) core_buttons |= WM_BUTTON_PLUS;
	if (buttons & BTN(BLOOPAIR_PRO_BUTTON_HOME)) core_buttons |= WM_BUTTON_HOME;
	if (buttons & BTN(BLOOPAIR_PRO_BUTTON_ONE)) core_buttons |= WM_BUTTON_ONE;
	if (buttons & BTN(BLOOPAIR_PRO_BUTTON_TWO)) core_buttons |= WM_BUTTON_TWO;

	return core_buttons;
}

static void encodeIRDot(uint8_t* out, int16_t x, int16_t y)
{
	x = CLAMP(x, 0, WIIMOTE_IR_MAX_X);
	y = CLAMP(y, 0, WIIMOTE_IR_MAX_Y);

	out[0] = x & 0xff;
	out[1] = y & 0xff;
	out[2] = ((x >> 8) & 0x3) | (((y >> 8) & 0x3) << 2) | ((WIIMOTE_IR_DOT_SIZE & 0xf) << 4);
	out[3] = 0xff;
	out[4] = 0xff;
}

static void encodeIRBlock(uint8_t* ir, int16_t x, int16_t y)
{
	memset(ir, 0xff, 12);
	encodeIRDot(&ir[0], x - WIIMOTE_IR_DOT_DISTANCE, y);
	encodeIRDot(&ir[5], x + WIIMOTE_IR_DOT_DISTANCE, y);
}

static uint8_t getWiimoteToggleButton(Controller* controller)
{
	WiimoteConfiguration* config = (WiimoteConfiguration*) controller->customConfig;
	if (!config) {
		return default_wiimote_configuration.irToggleButton;
	}

	return config->irToggleButton;
}

static uint8_t getWiimoteIRVelocity(Controller* controller)
{
	WiimoteConfiguration* config = (WiimoteConfiguration*) controller->customConfig;
	if (!config || config->irVelocity < 1) {
		return default_wiimote_configuration.irVelocity;
	}

	return CLAMP(config->irVelocity, 1, 100);
}

static void controllerDeinit_wiimote(Controller* controller)
{
	WiimoteData* wdata = (WiimoteData*) controller->wiimoteData;
	if (!wdata) {
		return;
	}

	controller->additionalData = wdata->physicalAdditionalData;
	if (wdata->physicalDeinit) {
		wdata->physicalDeinit(controller);
	}

	IOS_Free(LOCAL_PROCESS_HEAP_ID, wdata);
	controller->wiimoteData = NULL;
}

void controllerInit_wiimote(Controller* controller)
{
	void* physicalAdditionalData = controller->additionalData;
	ControllerDeinitFn physicalDeinit = controller->deinit;

	WiimoteData* wdata = (WiimoteData*) IOS_Alloc(LOCAL_PROCESS_HEAP_ID, sizeof(WiimoteData));
	if (!wdata) {
		return;
	}

	memset(wdata, 0, sizeof(WiimoteData));
	wdata->ir_x = WIIMOTE_IR_CENTER_X;
	wdata->ir_y = WIIMOTE_IR_CENTER_Y;
	wdata->physicalDeinit = physicalDeinit;
	wdata->physicalAdditionalData = physicalAdditionalData;

	controller->type = BLOOPAIR_CONTROLLER_WIIMOTE;
	Configuration_GetAll(controller->type, controller->bda,
		&controller->commonConfig, &controller->mapping,
		&controller->customConfig, &controller->customConfigSize);

	WiimoteConfiguration* config = (WiimoteConfiguration*) controller->customConfig;
	wdata->extensionMode = config ? config->extensionMode : WIIMOTE_EXTENSION_NONE;
	controller->wiimoteData = wdata;
	controller->deinit = controllerDeinit_wiimote;
	controller->dataReportingMode = WM_REPORT_ID_EXTENSION_DATA_REPORT;
}

void controllerSendInput_wiimote(Controller* controller)
{
	WiimoteData* wdata = (WiimoteData*) controller->wiimoteData;
	if (!wdata) {
		return;
	}

	BloopairReportBuffer repBuf;
	mapControllerInput(controller, &controller->reportBuffer, &repBuf);

	uint8_t toggleButton = getWiimoteToggleButton(controller);
	uint8_t togglePressed = (controller->reportBuffer.buttons & BTN(toggleButton)) != 0;
	if (togglePressed && !wdata->irToggleWasPressed) {
		wdata->irActive = !wdata->irActive;
	}
	wdata->irToggleWasPressed = togglePressed;

	uint16_t core_buttons = bswap16(wiimoteButtonsFromMappedButtons(repBuf.buttons));

	if (wdata->irActive) {
		int16_t right_x = controller->reportBuffer.right_stick_x;
		int16_t right_y = controller->reportBuffer.right_stick_y;
		uint8_t velocity = getWiimoteIRVelocity(controller);

		if (right_x > WIIMOTE_IR_DEADZONE || right_x < -WIIMOTE_IR_DEADZONE) {
			wdata->ir_x = CLAMP(wdata->ir_x + ((right_x * velocity) / WIIMOTE_IR_STEP_DIVISOR), 0, WIIMOTE_IR_MAX_X);
		}
		if (right_y > WIIMOTE_IR_DEADZONE || right_y < -WIIMOTE_IR_DEADZONE) {
			wdata->ir_y = CLAMP(wdata->ir_y + ((right_y * velocity) / WIIMOTE_IR_STEP_DIVISOR), 0, WIIMOTE_IR_MAX_Y);
		}

		WMCoreAccelIRReport report;
		memset(&report, 0, sizeof(report));
		report.report_id = 0x33;
		report.core_buttons = core_buttons;
		encodeIRBlock(report.ir, wdata->ir_x, wdata->ir_y);
		sendInputData(controller->handle, &report, sizeof(report));
	} else {
		WMCoreReport report;
		memset(&report, 0, sizeof(report));
		report.report_id = 0x30;
		report.core_buttons = core_buttons;
		sendInputData(controller->handle, &report, sizeof(report));
	}
}

void controllerModuleInit_wiimote(void)
{
	Configuration_SetFallback(BLOOPAIR_CONTROLLER_WIIMOTE, NULL, &default_wiimote_mapping, &default_wiimote_configuration, sizeof(default_wiimote_configuration));
}
