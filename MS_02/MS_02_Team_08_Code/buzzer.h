#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

// Buzzer driver (PWM). Works with passive buzzers (any pitch) and
// active buzzers (they just beep at their own fixed pitch).
#define BUZZER_PIN 15   // GP15, pin 20

void buzzer_init(void);
void buzzer_tone(uint16_t freq_hz);              // start a tone
void buzzer_off(void);
void buzzer_beep(uint16_t freq_hz, uint16_t ms); // blocking beep

void buzzer_ready_tone(void);   // short rising tone
void buzzer_fail_tones(void);   // rapid failure tones (YOU LOSE)

#endif
