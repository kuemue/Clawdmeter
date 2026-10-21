#include "board.h"
#include <Wire.h>

// Shared I2C bus for the FT6336 touch controller. No IO expander gates the
// panel; the backlight pin is configured in display_hal_begin().
extern "C" void board_init(void) {
    Wire.begin(TP_SDA, TP_SCL, 400000);
}
