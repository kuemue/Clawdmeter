#include "../../hal/board_caps.h"
#include "board.h"

// Width and height follow the saved quadrant: odd quadrants are landscape.
const BoardCaps& board_caps(void) {
    static BoardCaps caps;
    static bool ready = false;
    if (!ready) {
        const bool landscape = es3_quadrant() & 1;
        caps = {
            .name = BOARD_NAME,
            .width  = landscape ? PANEL_HEIGHT : PANEL_WIDTH,
            .height = landscape ? PANEL_WIDTH  : PANEL_HEIGHT,
            .round_panel = false,
            .button_count = 0,       // BOOT is the PWR role (power.cpp)
            .has_rotation = (bool)BOARD_HAS_ROTATION,
            .has_battery  = (bool)BOARD_HAS_BATTERY,
            .has_imu      = (bool)BOARD_HAS_IMU,
            // Meters and a still corner Clawd, as on the Freenove panel: the
            // full UI panics ipc0 at boot here (stack canary in BT init).
            .minimal_ui   = true,
            .has_pwr_button = true,  // BOOT: brightness, hold-to-pair
            .has_touch = true,
        };
        ready = true;
    }
    return caps;
}
