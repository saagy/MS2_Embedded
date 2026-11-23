#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

// Hobby servo driver (e.g. TowerPro SG90 micro servo).
//   Brown  -> GND (common ground)
//   Red    -> +5 V (L298N 5 V pin / terminal, NOT 12 V)
//   Orange -> signal pin below (PWM, 50 Hz)
// GP28 has PWM slice 6 to itself, so the 50 Hz servo signal does not
// disturb the RGB LED or motor PWM.
#define SERVO_PIN 28        // GP28, pin 34 (PWM slice 6A)

// Pulse widths for 0 and 180 degrees. SG90s vary a little: if the servo
// buzzes or hits a hard stop at the ends, move these closer together.
#define SERVO_MIN_US 500
#define SERVO_MAX_US 2400

void    servo_init(uint8_t pin);
void    servo_write(uint8_t angle);   // 0..180 degrees
void    servo_write_signed(int deg);  // -90..+90 degrees, 0 = middle
void    servo_off(void);              // stop pulses (servo goes limp)
uint8_t servo_get(void);

#endif
