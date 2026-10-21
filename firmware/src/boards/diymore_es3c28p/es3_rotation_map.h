#pragma once
#include <stdint.h>

// Map a touch point in native portrait panel coordinates (W x H, rotation 0)
// to UI coordinates for quadrant q, matching Arduino_GFX setRotation(q).
// Pure so it can be host-tested (test_es3c28p_rotation).

static inline void es3_touch_map(uint8_t q, uint16_t W, uint16_t H,
                                 uint16_t nx, uint16_t ny,
                                 uint16_t* x, uint16_t* y) {
    switch (q & 3) {
    case 1:  *x = ny;         *y = W - 1 - nx; break;   // 90° CW, landscape
    case 2:  *x = W - 1 - nx; *y = H - 1 - ny; break;   // 180°
    case 3:  *x = H - 1 - ny; *y = nx;         break;   // 270° CW, landscape
    default: *x = nx;         *y = ny;         break;
    }
}
