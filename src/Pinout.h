#ifndef PINOUT_INC
#define PINOUT_INC

// NODEMCU based on ESP8266

#define GPIO0_PIN 0
#define GPIO1_PIN 1
#define GPIO2_PIN 2
#define GPIO3_PIN 3
#define GPIO4_PIN 4
#define GPIO5_PIN 5
#define GPIO6_PIN 6
#define GPIO7_PIN 7
#define GPIO8_PIN 8
#define GPIO9_PIN 9
#define GPIO10_PIN 10
#define GPIO11_PIN 11
#define GPIO12_PIN 12
#define GPIO13_PIN 13
#define GPIO14_PIN 14
#define GPIO15_PIN 15
#define GPIO16_PIN 16
#define A0_PIN 17

// NODEMCU based on ESP8266 (human names)

#define PIN_D0 GPIO16_PIN // will break deep sleep mode?
#define PIN_D1 GPIO5_PIN
#define PIN_D2 GPIO4_PIN
#define PIN_D3 GPIO0_PIN
#define PIN_D4 GPIO2_PIN
#define PIN_D5 GPIO14_PIN
#define PIN_D6 GPIO12_PIN // using it breaks serial communication?
#define PIN_D7 GPIO13_PIN // RXD2
#define PIN_D8 GPIO15_PIN // TXD2, if used may break deep sleep mode?
#define PIN_D9 GPIO3_PIN // RXDO, if used will break serial communication
#define PIN_D10 GPIO1_PIN // TXDO, if used will break serial communication
#define PIN_D11 GPIO9_PIN // SDD2 / SD2
#define PIN_D12 GPIO10_PIN // SDD3 / SD3

// LEDS
#define LED0_PIN PIN_D0
#define LED1_PIN PIN_D1

// SERVO
#define SERVO0_PIN PIN_D6

// I2C OLED 128x64
// (Kuman 0.96inches I2C OLED 128x64 LCD screen)
#define LCD_SCL_PIN PIN_D1
#define LCD_SDA_PIN PIN_D2
#define LCD_RESET_PIN GPIO0_PIN

#endif // PINOUT_INC
