#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdint.h>

// RGB LED driver (PWM, so each colour has 0..255 brightness).
#define RGB_PIN_R 10   // GP10, pin 14
#define RGB_PIN_G 11   // GP11, pin 15
#define RGB_PIN_B 12   // GP12, pin 16

// Set to 1 if the LED's long leg goes to 3V3 (common anode),
// 0 if it goes to GND (common cathode).
#define RGB_COMMON_ANODE 0

void rgb_init(void);
void rgb_set(uint8_t r, uint8_t g, uint8_t b);
void rgb_off(void);

#endif
