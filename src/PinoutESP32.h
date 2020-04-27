#ifndef PINOUT_INC
#define PINOUT_INC

#include <pinouts/PinoutESP32.h> 

// I2C OLED 128x64
// (Kuman 0.96inches I2C OLED 128x64 LCD screen)
#define LCD_SCL_PIN GPIO1_PIN
#define LCD_SDA_PIN GPIO2_PIN

// LEDS

#ifndef LEDY_PIN            // 1 off, 0 on
#define LEDY_PIN GPIO27_PIN // YELLOW LED
#endif                      // LEDY_PIN

#ifndef LEDW_PIN            // 1 off, 0 on
#define LEDW_PIN GPIO14_PIN // WHITE LED
#endif                      // LEDW_PIN
#ifndef LEDR_PIN            // 1 off, 0 on
#define LEDR_PIN GPIO13_PIN  // RED LED
#endif                      // LEDR_PIN

// FAN
#define FAN_PIN GPIO12_PIN   // 0 off, 1 on

// SERVOS
#define SERVO0_PIN GPIO35_PIN
#define SERVO1_PIN GPIO34_PIN

// BUTTON
#define BUTTON0_PIN GPIO33_PIN

#endif // PINOUT_INC
