#pragma once
#include <cstdint>

// Motor PinsConfig
constexpr uint8_t STEP_PIN_A = 19;
constexpr uint8_t DIR_PIN_A  = 4;
constexpr uint8_t STEP_PIN_B = 18;
constexpr uint8_t DIR_PIN_B  = 5;
constexpr uint8_t EN_PIN     = 23;

// Driver UART
constexpr uint8_t DRIVER_TX_PIN = 17;
constexpr uint8_t DRIVER_RX_PIN = 16;
constexpr float R_SENSE = 0.11f;

// Lcd Display PinsConfig
constexpr uint8_t LCD_I2C_ADDRESS = 0x27;
constexpr uint8_t LCD_SDA_PIN = 21;
constexpr uint8_t LCD_SCL_PIN = 22;

// Rotary Encoder PinsConfig
constexpr uint8_t ENCODER_DT_PIN = 26;
constexpr uint8_t ENCODER_CLK_PIN = 27;
constexpr uint8_t ENCODER_SW_PIN = 25;

// buzzer
constexpr uint8_t BUZZER_PIN = 14;

// servo
constexpr uint8_t SERVO_PIN = 13;