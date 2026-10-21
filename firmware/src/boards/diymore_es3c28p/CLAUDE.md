# DIYMORE ES3C28P — `diymore_es3c28p`

Hardware notes for this board. Shared firmware rules are in `firmware/CLAUDE.md`.

DIYMORE / LCDWIKI **ES3C28P** (2.8" ILI9341V 240×320 IPS over plain 4-wire SPI, FT6336G capacitive touch on I2C, ESP32-S3 N16R8: 16 MB flash + 8 MB octal PSRAM). Build env: `diymore_es3c28p`. The no-touch twin is the ES3N28P. Pins are from the LCDWIKI pin table and match the Freenove FNK0104 panel wiring (this folder started as a copy of `freenove_s3_display_nontouch`).

- ILI9341 with inversion on (`ips = true`), no reset line (tied to EN), backlight LEDC PWM on GPIO 45.
- **Touch:** FT6336G at 0x38, SDA 16 / SCL 15, RST 18, INT 17 (INT unused; polled at 50 Hz). Raw points are native portrait and go through `es3_rotation_map.h` (host-tested in `test/test_es3c28p_rotation`).
- **No IMU, so rotation is button-driven.** BOOT (GPIO 0) is the only button and keeps the PWR role (short → brightness, hold ~1.5 s → pair); a **double-press** turns the UI 90° CW, and so does holding a still finger anywhere on the screen for 3 s (`touch.cpp`, since BOOT sits on the back). The quadrant is saved to NVS and the board restarts, because portrait and landscape need different layouts (`caps.cpp` sizes the layout from it, `display.cpp` passes it to the ILI9341 as its rotation). A lone short press waits 350 ms before stepping brightness.
- **`minimal_ui = true`.** The full UI panics ipc0 at boot on this board (stack canary during BT init), reproducible with the serial port open; minimal mode boots clean.
- **Not wired:** the speaker/mic codec (I2S on IO4–8, amp enable IO1 active LOW), the RGB LED (IO42), SD card, battery ADC (IO9).
