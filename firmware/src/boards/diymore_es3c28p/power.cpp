#include "../../hal/power_hal.h"
#include "board.h"
#include <Arduino.h>

// No PMU and no battery gauge. BOOT (GPIO0, active LOW) is the only button
// and takes the PWR role, plus rotation since there is no IMU:
//   short    - brightness, fired once no second press follows within
//              PWR_DOUBLE_MS (so a double-press never also steps brightness)
//   double   - rotate the UI 90° CW and restart (es3_rotate_next)
//   long     - fired once when a hold crosses PWR_LONG_MS (pair)
//   release  - fired on every release edge

#define PWR_POLL_MS   20
#define PWR_LONG_MS   1500
#define PWR_DOUBLE_MS 350

static bool     pwr_pressed_flag  = false;
static bool     pwr_long_flag     = false;
static bool     pwr_released_flag = false;
static bool     last_pwr_state    = false;
static bool     pwr_long_fired    = false;
static bool     short_pending     = false;   // one short press waiting for a second
static uint32_t pwr_press_started_ms = 0;
static uint32_t short_released_ms = 0;
static uint32_t last_pwr_ms       = 0;

void power_hal_init(void) {
    pinMode(BTN_PWR_GPIO, INPUT_PULLUP);
}

void power_hal_tick(void) {
    uint32_t now = millis();
    if (now - last_pwr_ms < PWR_POLL_MS) return;
    last_pwr_ms = now;

    bool pwr_now = (digitalRead(BTN_PWR_GPIO) == LOW);   // active LOW
#ifdef TOUCH_DEBUG
    if (pwr_now != last_pwr_state) Serial.printf("BOOT %s\n", pwr_now ? "down" : "up");
#endif
    if (pwr_now && !last_pwr_state) {                    // press edge
        pwr_press_started_ms = now;
        pwr_long_fired = false;
    } else if (pwr_now && last_pwr_state) {              // held
        if (!pwr_long_fired && (now - pwr_press_started_ms >= PWR_LONG_MS)) {
            pwr_long_flag  = true;
            pwr_long_fired = true;
            short_pending  = false;
        }
    } else if (!pwr_now && last_pwr_state) {             // release edge
        pwr_released_flag = true;
        if (!pwr_long_fired) {
            if (short_pending) es3_rotate_next();        // second short press; does not return
            short_pending = true;
            short_released_ms = now;
        }
    }
    if (short_pending && !pwr_now && now - short_released_ms >= PWR_DOUBLE_MS) {
        short_pending    = false;
        pwr_pressed_flag = true;                         // lone short press
    }
    last_pwr_state = pwr_now;
}

int  power_hal_battery_pct(void) { return -1; }
bool power_hal_is_charging(void) { return false; }
bool power_hal_is_vbus_in(void)  { return false; }

bool power_hal_pwr_pressed(void) {
    if (pwr_pressed_flag) { pwr_pressed_flag = false; return true; }
    return false;
}

bool power_hal_pwr_long_pressed(void) {
    if (pwr_long_flag) { pwr_long_flag = false; return true; }
    return false;
}

bool power_hal_pwr_released(void) {
    if (pwr_released_flag) { pwr_released_flag = false; return true; }
    return false;
}
