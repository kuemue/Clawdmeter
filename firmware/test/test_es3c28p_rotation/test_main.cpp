// Host unit test for the ES3C28P touch mapping: a native-portrait touch point
// must land inside the rotated UI and keep its position on the glass.
//
//   g++ -std=c++17 -I ../../src/boards/diymore_es3c28p test_main.cpp -o t && ./t

#include "es3_rotation_map.h"
#include <cstdio>

static int failures = 0;
#define CHECK(cond)                                                            \
    do {                                                                       \
        if (!(cond)) {                                                         \
            printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);             \
            ++failures;                                                        \
        }                                                                      \
    } while (0)

static const uint16_t W = 240, H = 320;

static void should_stay_on_screen_and_be_one_to_one() {
    for (uint8_t q = 0; q < 4; q++) {
        const uint16_t uw = (q & 1) ? H : W, uh = (q & 1) ? W : H;
        static bool seen[320][320];
        for (auto& r : seen) for (bool& b : r) b = false;
        bool ok = true;
        for (uint16_t ny = 0; ny < H && ok; ny++)
            for (uint16_t nx = 0; nx < W && ok; nx++) {
                uint16_t x, y;
                es3_touch_map(q, W, H, nx, ny, &x, &y);
                ok = x < uw && y < uh && !seen[y][x];
                if (ok) seen[y][x] = true;
            }
        CHECK(ok);
    }
}

static void should_follow_arduino_gfx_rotation() {
    uint16_t x, y;
    // Native top-left corner (by the panel's x=0,y=0 origin).
    es3_touch_map(0, W, H, 0, 0, &x, &y);  CHECK(x == 0 && y == 0);
    es3_touch_map(1, W, H, 0, 0, &x, &y);  CHECK(x == 0 && y == W - 1);
    es3_touch_map(2, W, H, 0, 0, &x, &y);  CHECK(x == W - 1 && y == H - 1);
    es3_touch_map(3, W, H, 0, 0, &x, &y);  CHECK(x == H - 1 && y == 0);
}

int main() {
    should_stay_on_screen_and_be_one_to_one();
    should_follow_arduino_gfx_rotation();
    if (failures) { printf("%d failure(s)\n", failures); return 1; }
    printf("OK\n");
    return 0;
}
