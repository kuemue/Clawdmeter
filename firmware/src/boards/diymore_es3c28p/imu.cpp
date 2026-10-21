#include "../../hal/imu_hal.h"
#include "board.h"
#include <Arduino.h>
#include <Preferences.h>

// No IMU. The quadrant comes from NVS and changes on a BOOT double-press
// (power.cpp). It is read lazily because board_caps() needs it for the
// layout size, possibly before imu_hal_init() runs.

static int8_t quadrant = -1;

uint8_t es3_quadrant(void) {
    if (quadrant < 0) {
        Preferences prefs;
        prefs.begin("rotation", true);
        quadrant = prefs.getUChar("q", 0) & 3;
        prefs.end();
    }
    return (uint8_t)quadrant;
}

void es3_rotate_next(void) {
    Preferences prefs;
    prefs.begin("rotation", false);
    prefs.putUChar("q", (es3_quadrant() + 1) & 3);
    prefs.end();
    Serial.println("Rotation saved; restarting into the new layout");
    delay(50);
    ESP.restart();
}

void    imu_hal_init(void) { Serial.printf("Rotation: %d (saved)\n", es3_quadrant()); }
void    imu_hal_tick(void) {}
uint8_t imu_hal_rotation_quadrant(void) { return es3_quadrant(); }
