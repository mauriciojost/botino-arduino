#ifndef PINOUT_INC
#define PINOUT_INC

#include <pinouts/PinoutESP32DevKit.h> 

// I2C OLED 128x64
// (Kuman 0.96inches I2C OLED 128x64 LCD screen)
#define LCD_SCL_PIN GPIO22_PIN
#define LCD_SDA_PIN GPIO21_PIN

// LEDS

#ifndef LEDY_PIN            // 1 off, 0 on
#define LEDY_PIN GPIO12_PIN // YELLOW LED
#endif                      // LEDY_PIN

#ifndef LEDW_PIN            // 1 off, 0 on
#define LEDW_PIN GPIO13_PIN // WHITE LED
#endif                      // LEDW_PIN
#ifndef LEDR_PIN            // 1 off, 0 on
#define LEDR_PIN GPIO14_PIN  // RED LED
#endif                      // LEDR_PIN

// FAN
#define FAN_PIN GPIO15_PIN   // 0 off, 1 on

// SERVOS
#define SERVO0_PIN GPIO33_PIN
#define SERVO1_PIN GPIO32_PIN

// BUTTON
#define BUTTON0_PIN GPIO27_PIN

#endif // PINOUT_INC
