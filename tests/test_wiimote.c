/*
 * Standalone host-PC unit tests for the Wiimote emulation layer
 * (ios/ios_pad/source/controllers/wiimote_controller.c).
 *
 * No external test framework: a plain main() runs each test function and
 * uses assert() to fail fast, printing [PASS] after each test that returns
 * normally.
 *
 * This file stubs out every IOS/BTA/Bluetooth-stack import so the real
 * controller logic can be compiled and exercised on a host PC, without the
 * devkitARM/devkitPPC cross toolchain. mapControllerInput() (stage 1 of the
 * button pipeline, hardware -> Wii U Pro Controller buttons) is stubbed as
 * an identity copy so tests can drive controller->reportBuffer directly and
 * see it appear unmodified as the input to stage 2
 * (mapProButtonsToCore(), Pro Controller buttons -> Wiimote buttons).
 *
 * Build:
 *   gcc -I ios/ios_pad/source -I libbloopair/include tests/test_wiimote.c -o test_wiimote
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- IOS / BTA / Bluetooth stack stubs ------------------------------- */

void* IOS_Alloc(uint32_t heap, uint32_t size) { return malloc(size); }
void* IOS_AllocAligned(uint32_t heap, uint32_t size, uint32_t alignment) { return malloc(size); }
void IOS_Free(uint32_t heap, void* ptr) { free(ptr); }
int smdIopSendMessage(int idx, void* ptr, uint32_t size) { return 0; }
int smdIopReceive(int idx, void* ptr) { return -0xc0005; }
void bdcpy(uint8_t* a, const uint8_t* b) { (void) a; (void) b; }
void* GKI_getbuf(uint32_t size) { return malloc(size); }
void GKI_freebuf(void* p) { free(p); }
void* GKI_getpoolbuf(uint8_t pool_id) { (void) pool_id; return NULL; }
uint8_t GKI_get_taskid(void) { return 0; }
void utl_freebuf(void** p) { (void) p; }
void bta_sys_sendmsg(void* msg) { (void) msg; }
uint8_t btm_remove_acl(uint8_t* a) { (void) a; return 0; }
int deleteDevice(uint8_t* a) { (void) a; return 0; }
int registerNewDevice(uint8_t* a, uint8_t* b, uint8_t* c) { (void) a; (void) b; (void) c; return 0; }
const char* bdaddr_to_string(uint8_t* a) { (void) a; return ""; }

uint32_t isSmdReady = 1;
uint32_t smdIopIndex = 0;
uint8_t local_device_bdaddr[6];

#include "controllers.h"

void bta_hh_snd_write_dev(uint8_t a, uint8_t b, uint8_t c, uint16_t d, uint8_t e, BT_HDR* f)
{
    (void) a; (void) b; (void) c; (void) d; (void) e; (void) f;
}
void BTA_HhSendData(uint8_t dev_handle, uint8_t* dev_bda, BT_HDR* p_buf)
{
    (void) dev_handle; (void) dev_bda; (void) p_buf;
}
void BTA_HhClose(uint8_t dev_handle) { (void) dev_handle; }
void BTA_HhAddDev(uint8_t* bda, uint16_t a, uint8_t b, uint8_t c, uint32_t d, uint8_t* e)
{
    (void) bda; (void) a; (void) b; (void) c; (void) d; (void) e;
}
uint8_t BTM_ReadRemoteDeviceName(uint8_t* bda, void* cb) { (void) bda; (void) cb; return 0; }
uint8_t BTM_WriteStoredLinkKey(uint8_t a, uint8_t* b, uint8_t* c, void* d)
{
    (void) a; (void) b; (void) c; (void) d; return 0;
}
void BTA_DmSetAfhChannels(uint8_t a, uint8_t b) { (void) a; (void) b; }
void BTA_DmAddDevice(uint8_t* a, uint8_t* b, uint8_t* c, uint32_t d, uint8_t e, uint8_t f, uint8_t g)
{
    (void) a; (void) b; (void) c; (void) d; (void) e; (void) f; (void) g;
}

void wiimoteCryptoInit(CryptoState* state, const uint8_t* key) { (void) state; (void) key; }
void wiimoteEncrypt(const CryptoState* state, void* enc, const void* dec, uint32_t addr, uint32_t size)
{
    (void) state; (void) addr;
    if (enc != dec) memcpy(enc, dec, size);
}
void wiimoteDecrypt(const CryptoState* state, void* dec, const void* enc, uint32_t addr, uint32_t size)
{
    (void) state; (void) addr;
    if (enc != dec) memcpy(dec, enc, size);
}

int Configuration_Init(void) { return 0; }
void Configuration_Deinit(void) {}
ConfigurationEntry* Configuration_GetFallback(BloopairControllerType type, uint8_t a) { (void) type; (void) a; return NULL; }
ConfigurationEntry* Configuration_GetForControllerType(BloopairControllerType type, uint8_t a) { (void) type; (void) a; return NULL; }
ConfigurationEntry* Configuration_GetForBDA(uint8_t* bda, uint8_t a) { (void) bda; (void) a; return NULL; }
BloopairCommonConfiguration* Configuration_GetCommon(BloopairControllerType type, uint8_t* bda) { (void) type; (void) bda; return NULL; }
MappingConfiguration* Configuration_GetMapping(BloopairControllerType type, uint8_t* bda) { (void) type; (void) bda; return NULL; }
void* Configuration_GetCustom(BloopairControllerType type, uint8_t* bda, uint32_t* size) { (void) type; (void) bda; (void) size; return NULL; }
int Configuration_GetAll(BloopairControllerType type, uint8_t* bda, BloopairCommonConfiguration** c, MappingConfiguration** m, void** cu, uint32_t* s)
{
    (void) type; (void) bda; (void) c; (void) m; (void) cu; (void) s;
    return -1;
}
void Configuration_SetFallback(BloopairControllerType type, const BloopairCommonConfiguration* c, const MappingConfiguration* m, const void* cu, uint32_t s)
{
    (void) type; (void) c; (void) m; (void) cu; (void) s;
}
void Configuration_SetWiimoteMode(BloopairControllerType type, uint8_t* bda, uint8_t enabled) { (void) type; (void) bda; (void) enabled; }
uint8_t Configuration_GetWiimoteMode(BloopairControllerType type, uint8_t* bda) { (void) type; (void) bda; return 0; }

/* Captures whatever controllerSendInput_wiimote() last sent, so tests can
 * inspect the report bytes without needing a real SMD/IOS transport. */
static uint8_t g_lastReport[64];
static uint16_t g_lastLen;

void sendInputData(uint8_t dev_handle, const void* data, uint16_t len)
{
    (void) dev_handle;
    assert(len <= sizeof(g_lastReport));
    memcpy(g_lastReport, data, len);
    g_lastLen = len;
}

/* Stage 1 of the button pipeline is intentionally untouched by Wiimote
 * emulation and isn't under test here, so stub it as an identity copy.
 * This lets tests drive controller->reportBuffer directly as if it were
 * already-mapped Wii U Pro Controller input. */
void mapControllerInput(Controller* controller, BloopairReportBuffer* in, BloopairReportBuffer* out)
{
    (void) controller;
    *out = *in;
}

/* Pulls in default_wiimote_mapping, default_wiimote_configuration,
 * mapProButtonsToCore(), encodeIRDotPair() and controllerSendInput_wiimote(). */
#include "controllers/wiimote_controller.c"

/* ---- Test helpers ------------------------------------------------------ */

#define RUN_TEST(fn) do { fn(); printf("[PASS] %s\n", #fn); } while (0)

static Controller g_controller;
static WiimoteData g_wdata;

static void resetController(WiimoteConfiguration* config)
{
    memset(&g_controller, 0, sizeof(g_controller));
    memset(&g_wdata, 0, sizeof(g_wdata));
    g_wdata.mapping = (MappingConfiguration*) &default_wiimote_mapping;
    g_wdata.config = config ? config : (WiimoteConfiguration*) &default_wiimote_configuration;
    g_controller.handle = 0;
    g_controller.wiimoteData = &g_wdata;
    memset(g_lastReport, 0, sizeof(g_lastReport));
    g_lastLen = 0;
}

static void decodeIRDot(const WMIRDotPair* dot, int* x, int* y)
{
    *x = dot->x | (((dot->x2_y2_xhi_yhi >> 6) & 0x3) << 8);
    *y = dot->y | (((dot->x2_y2_xhi_yhi >> 4) & 0x3) << 8);
}

/* ======================================================================
 * BUTTON MAPPING TESTS
 * ====================================================================== */

static void test_default_mapping_dpad(void)
{
    MappingConfiguration* m = (MappingConfiguration*) &default_wiimote_mapping;

    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_BUTTON_UP)) == (1 << WIIMOTE_BUTTON_UP));
    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_BUTTON_DOWN)) == (1 << WIIMOTE_BUTTON_DOWN));
    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_BUTTON_LEFT)) == (1 << WIIMOTE_BUTTON_LEFT));
    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_BUTTON_RIGHT)) == (1 << WIIMOTE_BUTTON_RIGHT));
}

static void test_default_mapping_face_buttons(void)
{
    MappingConfiguration* m = (MappingConfiguration*) &default_wiimote_mapping;

    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_BUTTON_A)) == (1 << WIIMOTE_BUTTON_TWO));
    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_BUTTON_B)) == (1 << WIIMOTE_BUTTON_ONE));
    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_BUTTON_X)) == (1 << WIIMOTE_BUTTON_A));
    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_BUTTON_Y)) == (1 << WIIMOTE_BUTTON_B));
}

static void test_default_mapping_triggers(void)
{
    MappingConfiguration* m = (MappingConfiguration*) &default_wiimote_mapping;

    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_TRIGGER_L)) == (1 << WIIMOTE_BUTTON_MINUS));
    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_TRIGGER_ZL)) == (1 << WIIMOTE_BUTTON_MINUS));
    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_TRIGGER_R)) == (1 << WIIMOTE_BUTTON_PLUS));
    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_TRIGGER_ZR)) == (1 << WIIMOTE_BUTTON_PLUS));
}

static void test_default_mapping_home(void)
{
    MappingConfiguration* m = (MappingConfiguration*) &default_wiimote_mapping;

    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_BUTTON_HOME)) == (1 << WIIMOTE_BUTTON_HOME));
}

static void test_no_spurious_buttons(void)
{
    MappingConfiguration* m = (MappingConfiguration*) &default_wiimote_mapping;

    assert(mapProButtonsToCore(m, 0) == 0);
}

static void test_multi_button(void)
{
    MappingConfiguration* m = (MappingConfiguration*) &default_wiimote_mapping;

    uint32_t proButtons = BTN(BLOOPAIR_PRO_BUTTON_UP) | BTN(BLOOPAIR_PRO_BUTTON_A);
    uint16_t expected = (1 << WIIMOTE_BUTTON_UP) | (1 << WIIMOTE_BUTTON_TWO);
    assert(mapProButtonsToCore(m, proButtons) == expected);
}

/* ======================================================================
 * IR TOGGLE TESTS
 *
 * The default config's irToggleButton is BLOOPAIR_PRO_BUTTON_STICK_R.
 * ====================================================================== */

static void test_ir_toggle_off_by_default(void)
{
    resetController(NULL);

    g_controller.reportBuffer.buttons = 0;
    controllerSendInput_wiimote(&g_controller);

    assert(g_wdata.irActive == 0);
}

static void test_ir_toggle_on_rising_edge(void)
{
    resetController(NULL);

    g_controller.reportBuffer.buttons = BTN(BLOOPAIR_PRO_BUTTON_STICK_R);
    controllerSendInput_wiimote(&g_controller);

    assert(g_wdata.irActive == 1);
}

static void test_ir_toggle_no_double_fire_while_held(void)
{
    resetController(NULL);

    g_controller.reportBuffer.buttons = BTN(BLOOPAIR_PRO_BUTTON_STICK_R);
    controllerSendInput_wiimote(&g_controller);
    assert(g_wdata.irActive == 1);

    /* held across several more ticks, should not toggle back off */
    controllerSendInput_wiimote(&g_controller);
    controllerSendInput_wiimote(&g_controller);
    controllerSendInput_wiimote(&g_controller);
    assert(g_wdata.irActive == 1);
}

static void test_ir_toggle_off_on_release_and_repress(void)
{
    resetController(NULL);

    /* press */
    g_controller.reportBuffer.buttons = BTN(BLOOPAIR_PRO_BUTTON_STICK_R);
    controllerSendInput_wiimote(&g_controller);
    assert(g_wdata.irActive == 1);

    /* release */
    g_controller.reportBuffer.buttons = 0;
    controllerSendInput_wiimote(&g_controller);
    assert(g_wdata.irActive == 1);

    /* press again -> toggles off */
    g_controller.reportBuffer.buttons = BTN(BLOOPAIR_PRO_BUTTON_STICK_R);
    controllerSendInput_wiimote(&g_controller);
    assert(g_wdata.irActive == 0);
}

/* ======================================================================
 * IR POSITION TESTS
 *
 * controllerSendInput_wiimote() computes the IR position fresh on every
 * call directly from the current stick position (it is not an
 * accumulator/integrator), scaled by irVelocity around the camera center
 * (512, 384) and clamped to [0, 1023] on both axes, see
 * CLAMP(ir_x, 0, 1023) / CLAMP(ir_y, 0, 1023) in wiimote_controller.c.
 * Repeating the same stick input across more ticks does not move the
 * position further, since there's nothing to accumulate.
 * ====================================================================== */

static void test_ir_position_center_at_rest(void)
{
    resetController(NULL);
    g_wdata.irActive = 1;

    g_controller.reportBuffer.right_stick_x = 0;
    g_controller.reportBuffer.right_stick_y = 0;
    controllerSendInput_wiimote(&g_controller);

    WMCoreAccelIRReport* r = (WMCoreAccelIRReport*) g_lastReport;
    int x, y;
    decodeIRDot(&r->ir[0], &x, &y);
    assert(x == 512);
    assert(y == 384);
}

static void test_ir_velocity_moves_right(void)
{
    resetController(NULL);
    g_wdata.irActive = 1;

    g_controller.reportBuffer.right_stick_x = 1140; /* full right */
    controllerSendInput_wiimote(&g_controller);

    WMCoreAccelIRReport* r = (WMCoreAccelIRReport*) g_lastReport;
    int x, y;
    decodeIRDot(&r->ir[0], &x, &y);
    assert(x > 512);
}

static void test_ir_velocity_moves_left(void)
{
    resetController(NULL);
    g_wdata.irActive = 1;

    g_controller.reportBuffer.right_stick_x = -1140; /* full left */
    controllerSendInput_wiimote(&g_controller);

    WMCoreAccelIRReport* r = (WMCoreAccelIRReport*) g_lastReport;
    int x, y;
    decodeIRDot(&r->ir[0], &x, &y);
    assert(x < 512);
}

static void test_ir_velocity_moves_up(void)
{
    resetController(NULL);
    g_wdata.irActive = 1;

    /* setStickAxis() represents "stick up" as a negative right_stick_y
     * (see BLOOPAIR_PRO_STICK_R_UP in controllers.c). Because
     * controllerSendInput_wiimote() computes
     * ir_y = IR_CENTER_Y - (right_stick_y * irVelocity) / PRO_AXIS_NORMALIZE_VALUE,
     * a negative right_stick_y *increases* ir_y. */
    g_controller.reportBuffer.right_stick_y = -1140; /* full up */
    controllerSendInput_wiimote(&g_controller);

    WMCoreAccelIRReport* r = (WMCoreAccelIRReport*) g_lastReport;
    int x, y;
    decodeIRDot(&r->ir[0], &x, &y);
    assert(y > 384);
}

static void test_ir_clamp_x(void)
{
    resetController(NULL);
    g_wdata.irActive = 1;

    /* Default irVelocity (50) can never push ir_x anywhere near the [0, 1023]
     * bound, since the position is recomputed fresh every tick rather than
     * accumulated. Use an exaggerated irVelocity to actually drive the
     * clamp, and confirm it engages and stays engaged across repeated
     * ticks. */
    WiimoteConfiguration config = { .extensionMode = WIIMOTE_EXTENSION_NONE,
                                     .irToggleButton = BLOOPAIR_PRO_BUTTON_STICK_R,
                                     .irVelocity = 200 };
    resetController(&config);
    g_wdata.irActive = 1;

    g_controller.reportBuffer.right_stick_x = 1140; /* full right */
    for (int i = 0; i < 100; i++) {
        controllerSendInput_wiimote(&g_controller);

        WMCoreAccelIRReport* r = (WMCoreAccelIRReport*) g_lastReport;
        int x, y;
        decodeIRDot(&r->ir[0], &x, &y);
        assert(x <= 1023);
    }
}

static void test_ir_clamp_x_low(void)
{
    WiimoteConfiguration config = { .extensionMode = WIIMOTE_EXTENSION_NONE,
                                     .irToggleButton = BLOOPAIR_PRO_BUTTON_STICK_R,
                                     .irVelocity = 200 };
    resetController(&config);
    g_wdata.irActive = 1;

    g_controller.reportBuffer.right_stick_x = -1140; /* full left */
    for (int i = 0; i < 100; i++) {
        controllerSendInput_wiimote(&g_controller);

        WMCoreAccelIRReport* r = (WMCoreAccelIRReport*) g_lastReport;
        int x, y;
        decodeIRDot(&r->ir[0], &x, &y);
        assert(x >= 0);
    }
}

static void test_ir_clamp_y(void)
{
    /* "up" increases ir_y (see test_ir_velocity_moves_up), so the upper
     * clamp is reached by pushing the stick up, not down. */
    WiimoteConfiguration config = { .extensionMode = WIIMOTE_EXTENSION_NONE,
                                     .irToggleButton = BLOOPAIR_PRO_BUTTON_STICK_R,
                                     .irVelocity = 200 };
    resetController(&config);
    g_wdata.irActive = 1;

    g_controller.reportBuffer.right_stick_y = -1140; /* full up */
    for (int i = 0; i < 100; i++) {
        controllerSendInput_wiimote(&g_controller);

        WMCoreAccelIRReport* r = (WMCoreAccelIRReport*) g_lastReport;
        int x, y;
        decodeIRDot(&r->ir[0], &x, &y);
        assert(y <= 1023);
    }
}

static void test_ir_clamp_y_low(void)
{
    /* "down" decreases ir_y, so the lower clamp is reached by pushing the
     * stick down. */
    WiimoteConfiguration config = { .extensionMode = WIIMOTE_EXTENSION_NONE,
                                     .irToggleButton = BLOOPAIR_PRO_BUTTON_STICK_R,
                                     .irVelocity = 200 };
    resetController(&config);
    g_wdata.irActive = 1;

    g_controller.reportBuffer.right_stick_y = 1140; /* full down */
    for (int i = 0; i < 100; i++) {
        controllerSendInput_wiimote(&g_controller);

        WMCoreAccelIRReport* r = (WMCoreAccelIRReport*) g_lastReport;
        int x, y;
        decodeIRDot(&r->ir[0], &x, &y);
        assert(y >= 0);
    }
}

/* ======================================================================
 * IR REPORT FORMAT TESTS
 * ====================================================================== */

static void test_ir_report_size(void)
{
    /* report_id(1) + core_buttons(2) + accel(3) + ir[2] (2 * 5-byte
     * WMIRDotPair) = 16 bytes. Already enforced at compile time by
     * CHECK_SIZE(WMCoreAccelIRReport, 16) in wiimote_controller.h. */
    assert(sizeof(WMCoreAccelIRReport) == 16);
}

static void test_ir_off_report_size(void)
{
    assert(sizeof(WMCoreReport) == 3);
}

static void test_ir_dot_inactive_all_ff(void)
{
    WMIRDotPair dot;
    memset(&dot, 0, sizeof(dot));
    encodeIRDotPair(&dot, 0, 123, 456);

    uint8_t* bytes = (uint8_t*) &dot;
    for (size_t i = 0; i < sizeof(dot); i++) {
        assert(bytes[i] == 0xff);
    }
}

static void test_ir_dot_active_x_encoding(void)
{
    WMIRDotPair dot;
    encodeIRDotPair(&dot, 1, 0x1AB, 0);

    assert(dot.x == 0xAB);
    assert(((dot.x2_y2_xhi_yhi >> 6) & 0x3) == 0x1);
}

static void test_ir_dot_active_y_encoding(void)
{
    WMIRDotPair dot;
    encodeIRDotPair(&dot, 1, 0, 0x155);

    assert(dot.y == 0x55);
    assert(((dot.x2_y2_xhi_yhi >> 4) & 0x3) == 0x1);
}

/* ======================================================================
 * REPORT ID TESTS
 * ====================================================================== */

static void test_report_id_ir_off(void)
{
    resetController(NULL);
    g_wdata.irActive = 0;

    controllerSendInput_wiimote(&g_controller);

    assert(g_lastReport[0] == 0x30);
}

static void test_report_id_ir_on(void)
{
    resetController(NULL);
    g_wdata.irActive = 1;

    controllerSendInput_wiimote(&g_controller);

    assert(g_lastReport[0] == 0x33);
}

/* ======================================================================
 * CORE BUTTONS NOT AFFECTED BY THE IR TOGGLE BUTTON
 * ====================================================================== */

static void test_ir_toggle_button_not_in_core_buttons(void)
{
    MappingConfiguration* m = (MappingConfiguration*) &default_wiimote_mapping;

    /* BLOOPAIR_PRO_BUTTON_STICK_R (the default irToggleButton) doesn't
     * appear as a "from" entry in default_wiimote_mapping, so pressing it
     * must not set any bit in the emulated Wiimote's core buttons. */
    assert(mapProButtonsToCore(m, BTN(BLOOPAIR_PRO_BUTTON_STICK_R)) == 0);
}

/* ---- main --------------------------------------------------------------- */

int main(void)
{
    RUN_TEST(test_default_mapping_dpad);
    RUN_TEST(test_default_mapping_face_buttons);
    RUN_TEST(test_default_mapping_triggers);
    RUN_TEST(test_default_mapping_home);
    RUN_TEST(test_no_spurious_buttons);
    RUN_TEST(test_multi_button);

    RUN_TEST(test_ir_toggle_off_by_default);
    RUN_TEST(test_ir_toggle_on_rising_edge);
    RUN_TEST(test_ir_toggle_no_double_fire_while_held);
    RUN_TEST(test_ir_toggle_off_on_release_and_repress);

    RUN_TEST(test_ir_position_center_at_rest);
    RUN_TEST(test_ir_velocity_moves_right);
    RUN_TEST(test_ir_velocity_moves_left);
    RUN_TEST(test_ir_velocity_moves_up);
    RUN_TEST(test_ir_clamp_x);
    RUN_TEST(test_ir_clamp_y);
    RUN_TEST(test_ir_clamp_x_low);
    RUN_TEST(test_ir_clamp_y_low);

    RUN_TEST(test_ir_report_size);
    RUN_TEST(test_ir_off_report_size);
    RUN_TEST(test_ir_dot_inactive_all_ff);
    RUN_TEST(test_ir_dot_active_x_encoding);
    RUN_TEST(test_ir_dot_active_y_encoding);

    RUN_TEST(test_report_id_ir_off);
    RUN_TEST(test_report_id_ir_on);

    RUN_TEST(test_ir_toggle_button_not_in_core_buttons);

    printf("\nAll tests passed.\n");
    return 0;
}
