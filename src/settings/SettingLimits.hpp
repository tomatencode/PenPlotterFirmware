#pragma once
#include <cstdint>
#include <cstddef>

// Name
constexpr size_t    SETTING_MAX_NAME_LEN                = 32;   // arbitrary limit for user-friendly names

// WiFi / Network
constexpr size_t    SETTING_MAX_SSID_LEN                = 32;   // IEEE 802.11 limit
constexpr size_t    SETTING_MAX_PASSWORD_LEN            = 64;   // WPA2 limit
constexpr size_t    SETTING_MAX_MDNS_LEN                = 63;   // DNS label limit

// Driver
constexpr float     MIN_DRIVER_CURRENT_MA               = 100.0f;
constexpr float     MAX_DRIVER_CURRENT_MA               = 2000.0f;  // TMC2209 max RMS
constexpr float     MIN_MICROSTEPS                      = 1.0f;
constexpr float     MAX_MICROSTEPS                      = 256.0f;   // TMC2209 max

// Feed rates
constexpr float     MIN_DRAW_FEED_RATE_MM_PER_S         = 1.0f;
constexpr float     MAX_DRAW_FEED_RATE_MM_PER_S         = 200.0f;
constexpr float     MIN_TRAVEL_FEED_RATE_MM_PER_S       = 1.0f;
constexpr float     MAX_TRAVEL_FEED_RATE_MM_PER_S       = 500.0f;

// Pen lift time
constexpr float     MIN_PEN_LIFT_TIME_S                = 0.01f;
constexpr float     MAX_PEN_LIFT_TIME_S                = 1.0f;

// Minimum feature size
constexpr float     MIN_MIN_FEATURE_SIZE_MM             = 0.1f;
constexpr float     MAX_MIN_FEATURE_SIZE_MM             = 10.0f;


// Homing
constexpr float     MIN_HOMING_SPEED_STP_PER_S          = 10.0f;
constexpr float     MAX_HOMING_SPEED_STP_PER_S          = 2000.0f;
constexpr float     MIN_HOMING_BO_SPEED_STP_PER_S       = 1.0f;
constexpr float     MAX_HOMING_BO_SPEED_STP_PER_S       = 1000.0f;
constexpr float     MIN_SG_THRESHOLD                    = 0.0f;
constexpr float     MAX_SG_THRESHOLD                    = 255.0f;   // 8-bit SGTHRS register
constexpr uint16_t  MIN_BACK_OFF_STEPS                  = 0;
constexpr uint16_t  MAX_BACK_OFF_STEPS                  = 1000;
constexpr uint32_t  MIN_HOMING_TIMEOUT_US               = 1'000'000UL;   // 1 s
constexpr uint32_t  MAX_HOMING_TIMEOUT_US               = 60'000'000UL;  // 60 s
constexpr uint16_t  MIN_SG_CHECK_INTERVAL_MS            = 10;
constexpr uint16_t  MAX_SG_CHECK_INTERVAL_MS            = 1000;
constexpr uint16_t  MIN_SG_START_TIMEOUT_MS             = 50;
constexpr uint16_t  MAX_SG_START_TIMEOUT_MS             = 5000;
constexpr uint8_t   MIN_SG_HISTORY_SIZE                 = 1;
constexpr uint8_t   MAX_SG_HISTORY_SIZE                 = 20;

// Pen
constexpr float     MIN_PEN_ANGLE_DEG                   = 0.0f;
constexpr float     MAX_PEN_ANGLE_DEG                   = 180.0f;
