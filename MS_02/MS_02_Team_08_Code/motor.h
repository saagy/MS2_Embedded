#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include <stdbool.h>

// DC motor driver for one channel of an L298N H-bridge.
//   ENx (speed)     <- PWM pin   (remove the ENA/ENB jumper on the L298N)
//   INx1 / INx2     <- direction pins
// Power: 3 x 3.7 V Li-ion in series (~11.1 V, 12.6 V full) -> L298N +12V.
// The L298N GND MUST be connected to a Pico GND (common ground).

// Motor X: L298N #1 channel A
#define MOTOR_X_EN   10   // GP10, pin 14 (PWM slice 5A)
#define MOTOR_X_IN1  11   // GP11, pin 15
#define MOTOR_X_IN2  12   // GP12, pin 16

// Motor Y: L298N #2 channel A
#define MOTOR_Y_EN    6   // GP6,  pin 9  (PWM slice 3A)
#define MOTOR_Y_IN1   7   // GP7,  pin 10
#define MOTOR_Y_IN2   8   // GP8,  pin 11

#define MOTOR_PWM_HZ 1000 // 1 kHz PWM on the enable pin

typedef struct {
    uint8_t  en, in1, in2;
    int8_t   speed;        // last speed set, -100..100
} motor_t;

void   motor_init(motor_t *m, uint8_t en, uint8_t in1, uint8_t in2);
void   motor_set(motor_t *m, int speed);   // -100..100 %, sign = direction
void   motor_stop(motor_t *m);             // coast (outputs off)
void   motor_brake(motor_t *m);            // short brake (both INs high)
int8_t motor_get(const motor_t *m);

#endif
