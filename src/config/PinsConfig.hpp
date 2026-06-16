#pragma once
#include <cstdint>

// Motor PinsConfig
constexpr uint8_t STEP_PIN_A = 27;
constexpr uint8_t DIR_PIN_A  = 26;
constexpr uint8_t STEP_PIN_B = 33;
constexpr uint8_t DIR_PIN_B  = 25;
constexpr uint8_t EN_PIN     = 14;

// Driver UART
constexpr uint8_t DRIVER_TX_PIN = 17;
constexpr uint8_t DRIVER_RX_PIN = 16;
constexpr float R_SENSE = 0.11f;

// Lcd Display PinsConfig
constexpr uint8_t LCD_I2C_ADDRESS = 0x27;
constexpr uint8_t LCD_SDA_PIN = 21;
constexpr uint8_t LCD_SCL_PIN = 22;

// Rotary Encoder PinsConfig
constexpr uint8_t ENCODER_DT_PIN = 35;
constexpr uint8_t ENCODER_CLK_PIN = 34;
constexpr uint8_t ENCODER_SW_PIN = 23;

// buzzer
constexpr uint8_t BUZZER_PIN = 32;

// servo
constexpr uint8_t SERVO_PIN = 4;