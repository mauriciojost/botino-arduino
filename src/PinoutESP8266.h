#ifndef PINOUT_INC
#define PINOUT_INC

#include <pinouts/PinoutESP8266.h>

// I2C OLED 128x64
// (Kuman 0.96inches I2C OLED 128x64 LCD screen)
#define LCD_SCL_PIN PIN_D1 // cannot be changed! (fixed in the library used)
#define LCD_SDA_PIN PIN_D2 // same as above

// LEDS

#ifndef LEDY_PIN        // 1 off, 0 on
#define LEDY_PIN PIN_D0 // YELLOW LED // used differently in deep sleep mode
#endif                  // LEDY_PIN

#ifndef LEDW_PIN        // 1 off, 0 on
#define LEDW_PIN PIN_D3 // WHITE LED
#endif                  // LEDW_PIN
#ifndef LEDR_PIN        // 1 off, 0 on
#define LEDR_PIN PIN_D4 // RED LED
#endif                  // LEDR_PIN

// FAN
#define FAN_PIN PIN_D6 // 0 off, 1 on

// SERVOS
#define SERVO0_PIN PIN_D7
#define SERVO1_PIN PIN_D8

// BUTTON
#define BUTTON0_PIN PIN_D5

#endif // PINOUT_INC
