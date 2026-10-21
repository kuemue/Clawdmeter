#include "../../hal/touch_hal.h"
#include "board.h"
#include "es3_rotation_map.h"
#include <Arduino.h>
#include <Wire.h>

// Minimal FT6336G I2C reader (FocalTech standard register layout):
//   reg 0x02: low nibble = active finger count
//   reg 0x03 / 0x04: X1 high (low nibble) + X1 low
//   reg 0x05 / 0x06: Y1 high (low nibble) + Y1 low
// Raw points are native portrait; es3_rotation_map.h turns them to UI
// coordinates. A still finger held ROTATE_HOLD_MS anywhere rotates the UI,
// so the back-mounted BOOT button isn't the only way.

#define ROTATE_HOLD_MS  3000
#define ROTATE_SLOP_PX  30
#define ROTATE_GAP_MS   250

static bool     touch_init_ok = false;
static bool     touch_pressed = false;
static uint16_t touch_x       = 0;
static uint16_t touch_y       = 0;
static uint32_t last_read_ms  = 0;
static uint32_t hold_start_ms = 0;   // 0 = no finger down
static uint32_t hold_seen_ms  = 0;   // last sample with the finger down
static uint16_t hold_x = 0, hold_y = 0;

static void rotate_hold_tick(uint32_t now) {
    if (!touch_pressed) {
        // The controller drops the odd sample mid-hold; only a real lift
        // (no finger for ROTATE_GAP_MS) ends the hold.
        if (hold_start_ms && now - hold_seen_ms > ROTATE_GAP_MS) hold_start_ms = 0;
        return;
    }
    hold_seen_ms = now;
    if (hold_start_ms == 0 || abs((int)touch_x - hold_x) > ROTATE_SLOP_PX ||
        abs((int)touch_y - hold_y) > ROTATE_SLOP_PX) {
#ifdef TOUCH_DEBUG
        Serial.printf("hold start at (%u,%u)\n", touch_x, touch_y);
#endif
        hold_start_ms = now ? now : 1;   // (re)start: new finger or it moved
        hold_x = touch_x;
        hold_y = touch_y;
    } else if (now - hold_start_ms >= ROTATE_HOLD_MS) {
        es3_rotate_next();   // does not return
    }
}

static void ft6336_read(void) {
    Wire.beginTransmission(TP_ADDR);
    Wire.write(0x02);
    if (Wire.endTransmission(false) != 0) { touch_pressed = false; return; }
    if (Wire.requestFrom((uint8_t)TP_ADDR, (uint8_t)5) != 5) { touch_pressed = false; return; }

    uint8_t fingers = Wire.read() & 0x0F;
    uint8_t xH = Wire.read();
    uint8_t xL = Wire.read();
    uint8_t yH = Wire.read();
    uint8_t yL = Wire.read();

    if (fingers == 0 || fingers > 2) { touch_pressed = false; return; }

    uint16_t nx = ((uint16_t)(xH & 0x0F) << 8) | xL;
    uint16_t ny = ((uint16_t)(yH & 0x0F) << 8) | yL;
    if (nx >= PANEL_WIDTH)  nx = PANEL_WIDTH - 1;
    if (ny >= PANEL_HEIGHT) ny = PANEL_HEIGHT - 1;
    es3_touch_map(es3_quadrant(), PANEL_WIDTH, PANEL_HEIGHT, nx, ny,
                  &touch_x, &touch_y);
    touch_pressed = true;
#ifdef TOUCH_DEBUG
    Serial.printf("touch raw=(%3u,%3u) -> (%3u,%3u)\n", nx, ny, touch_x, touch_y);
#endif
}

void touch_hal_init(void) {
    pinMode(TP_INT, INPUT);
    pinMode(TP_RST, OUTPUT);
    digitalWrite(TP_RST, LOW);
    delay(10);
    digitalWrite(TP_RST, HIGH);
    delay(300);   // controller boot

    Wire.beginTransmission(TP_ADDR);   // bus started in board_init()
    Wire.write(0xA5);   // power mode: active
    Wire.write(0x00);
    if (Wire.endTransmission() == 0) {
        touch_init_ok = true;
        Serial.println("FT6336 touch init OK");
    } else {
        Serial.printf("FT6336 not found at 0x%02X - touch disabled\n", TP_ADDR);
    }
}

void touch_hal_read(uint16_t* x, uint16_t* y, bool* pressed) {
    if (!touch_init_ok) { *x = 0; *y = 0; *pressed = false; return; }

    uint32_t now = millis();
    if (now - last_read_ms >= 20) {   // 50 Hz poll, one short I2C burst
        last_read_ms = now;
        ft6336_read();
        rotate_hold_tick(now);
    }
    *x = touch_x;
    *y = touch_y;
    *pressed = touch_pressed;
}
