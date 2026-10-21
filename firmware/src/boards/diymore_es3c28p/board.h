#pragma once
#include <stdint.h>

// DIYMORE / LCDWIKI ES3C28P - 2.8" 240x320 IPS TFT, ILI9341V over plain 4-wire
// SPI, FT6336G capacitive touch on I2C, ESP32-S3 N16R8 (16 MB flash + 8 MB
// octal PSRAM). Same panel wiring as the Freenove FNK0104 (pins below match
// the LCDWIKI ES3C28P pin table). No PMU, no IMU; backlight is LEDC PWM.
//
// No IMU, so orientation is button-driven: double-press BOOT turns the UI 90°
// CW, persisted to NVS. Portrait and landscape need different layouts, so the
// board restarts to apply it (imu.cpp).

#define BOARD_NAME           "DIYMORE ES3C28P"

// ---- Native panel geometry (portrait, rotation 0) ----
#define PANEL_WIDTH          240
#define PANEL_HEIGHT         320

// ---- SPI display pins (ILI9341V) ----
#define TFT_CS               10
#define TFT_MOSI             11
#define TFT_SCLK             12
#define TFT_MISO             13
#define TFT_BL               45    // backlight, LEDC PWM
#define TFT_DC               46
#define TFT_RST              -1    // tied to EN; software reset

// ---- Touch (FT6336G) ----
#define TP_SDA               16
#define TP_SCL               15
#define TP_RST               18
#define TP_INT               17
#define TP_ADDR              0x38

// ---- Buttons ----
#define BTN_PWR_GPIO         0     // BOOT - brightness, double-press rotate, hold-to-pair

// ---- Capability flags ----
#define BOARD_HAS_SECONDARY_BUTTON 0
#define BOARD_HAS_ROTATION         1   // button-driven, not IMU-driven
#define BOARD_HAS_IMU              0
#define BOARD_HAS_BATTERY          0
#define BOARD_HAS_IO_EXPANDER      0

// Saved UI quadrant (0..3, 90° CW steps); odd = landscape. imu.cpp.
uint8_t es3_quadrant(void);
// Advance the quadrant, save it and restart into the new layout.
void    es3_rotate_next(void);
