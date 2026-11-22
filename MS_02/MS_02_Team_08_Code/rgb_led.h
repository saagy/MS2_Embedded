#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdint.h>

// RGB LED driver (PWM, so each colour has 0..255 brightness).
#define RGB_PIN_R 16   // GP16, pin 21
#define RGB_PIN_G 17   // GP17, pin 22
#define RGB_PIN_B 18   // GP18, pin 24

// Set to 1 if the LED's long leg goes to 3V3 (common anode),
// 0 if it goes to GND (common cathode).
#define RGB_COMMON_ANODE 0

void rgb_init(void);
void rgb_set(uint8_t r, uint8_t g, uint8_t b);
void rgb_off(void);

#endif
