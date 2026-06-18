#pragma once
#include <cstdint>

// Pen
constexpr float PEN_UP_DEG = 70.0f;
constexpr float PEN_DOWN_DEG = 135.0f;

// Driver
constexpr float DRIVER_CURRENT_MA = 1000.0f;
constexpr uint16_t MICROSTEPS = 16;

// Homing
constexpr float HOMING_SPEED_STP_PER_S = 360.0f;
constexpr float STALLGUARD_THRESHOLD = 160.0f;
constexpr float HOMING_BACK_OFF_SPEED_STP_PER_S = 100.0f;
constexpr uint16_t BACK_OFF_STEPS_X = 20;
constexpr uint16_t BACK_OFF_STEPS_Y = 5;
constexpr uint32_t HOMING_TIMEOUT_US = 10000000UL;  // 10 seconds
constexpr uint16_t SG_CHECK_INTERVAL_MS = 50;
constexpr uint16_t SG_START_TIMEOUT_MS = 200;
constexpr uint8_t SG_HISTORY_SIZE = 3;  // Number of SG readings to average

// Movement Speeds
constexpr float FEED_RATE_DRAW_MM_PER_S = 30.0f;
constexpr float FEED_RATE_TRAVEL_MM_PER_S = 75.0f;

// pen lift/lower delay
constexpr float PEN_LIFT_TIME_S = 0.2f;

// Minimum feature size
constexpr float MIN_FEATURE_SIZE_MM = 1.0f;