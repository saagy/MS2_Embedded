#include "servo.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define SERVO_PERIOD_US 20000   // 50 Hz

static uint8_t servo_pin;
static uint8_t servo_angle;

void servo_init(uint8_t pin) {
    servo_pin = pin;
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    // 1 tick = 1 us, so the level is simply the pulse width in microseconds
    pwm_set_clkdiv(slice, (float)clock_get_hz(clk_sys) / 1000000.0f);
    pwm_set_wrap(slice, SERVO_PERIOD_US - 1);
    pwm_set_gpio_level(pin, 0);
    pwm_set_enabled(slice, true);
    servo_angle = 0;
}

void servo_write(uint8_t angle) {
    if (angle > 180) angle = 180;
    uint32_t us = SERVO_MIN_US +
                  (uint32_t)angle * (SERVO_MAX_US - SERVO_MIN_US) / 180u;
    pwm_set_gpio_level(servo_pin, (uint16_t)us);
    servo_angle = angle;
}

void servo_write_signed(int deg) {
    if (deg > 90)  deg = 90;
    if (deg < -90) deg = -90;
    servo_write((uint8_t)(deg + 90));   // -90 -> 0, 0 -> 90, +90 -> 180
}

void servo_off(void) {
    pwm_set_gpio_level(servo_pin, 0);
}

uint8_t servo_get(void) {
    return servo_angle;
}
