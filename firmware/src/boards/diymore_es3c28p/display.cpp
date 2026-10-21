#include "../../hal/display_hal.h"
#include "board.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>

// ILI9341V over plain 4-wire SPI, inversion on (ips = true) as on the
// Freenove panel. Rotation is the saved quadrant, applied by the controller
// (MADCTL), so no CPU strip rotation. Brightness is LEDC PWM on the backlight.

static Arduino_DataBus* bus = nullptr;
static Arduino_ILI9341* gfx = nullptr;

void display_hal_init(void) {
    bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
    gfx = new Arduino_ILI9341(bus, TFT_RST, es3_quadrant(), true /* ips */,
                              PANEL_WIDTH, PANEL_HEIGHT);
}

void display_hal_begin(void) {
    gfx->begin();
    gfx->fillScreen(0x0000);
    ledcAttach(TFT_BL, 5000 /* Hz */, 8 /* bits */);
    ledcWrite(TFT_BL, 200);
}

void display_hal_set_brightness(uint8_t level) {
    ledcWrite(TFT_BL, level);
}

void display_hal_draw_bitmap(int32_t x, int32_t y, int32_t w, int32_t h,
                             const uint16_t* pixels) {
    if (gfx) gfx->draw16bitRGBBitmap(x, y, (uint16_t*)pixels, w, h);
}

void display_hal_tick(void) {
    // Rotation is fixed per boot (see imu.cpp).
}

// SPI TFT: no flush-region alignment requirement.
void display_hal_round_area(int32_t* x1, int32_t* y1, int32_t* x2, int32_t* y2) {
    (void)x1; (void)y1; (void)x2; (void)y2;
}
