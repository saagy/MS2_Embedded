#ifndef POT_H
#define POT_H

#include <stdint.h>

// Potentiometer driver (analog input on the Pico ADC).
// Valid pins: GP26 (ADC0), GP27 (ADC1), GP28 (ADC2).

void     pot_init(uint8_t gpio);   // set up one pot pin
uint16_t pot_read(uint8_t gpio);   // raw value 0..4095 (averaged)
uint8_t  pot_percent(uint8_t gpio); // 0..100 %

#endif
