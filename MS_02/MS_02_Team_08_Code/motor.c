#include "motor.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define PWM_WRAP 999   // 1000 duty steps

void motor_init(motor_t *m, uint8_t en, uint8_t in1, uint8_t in2) {
    m->en = en; m->in1 = in1; m->in2 = in2; m->speed = 0;

    // Direction pins start LOW so the motor can't twitch at power-up
    gpio_init(in1); gpio_set_dir(in1, GPIO_OUT); gpio_put(in1, 0);
    gpio_init(in2); gpio_set_dir(in2, GPIO_OUT); gpio_put(in2, 0);

    gpio_set_function(en, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(en);
    float div = (float)clock_get_hz(clk_sys) / (MOTOR_PWM_HZ * (PWM_WRAP + 1));
    pwm_set_clkdiv(slice, div);
    pwm_set_wrap(slice, PWM_WRAP);
    pwm_set_gpio_level(en, 0);
    pwm_set_enabled(slice, true);
}

void motor_set(motor_t *m, int speed) {
    if (speed > 100)  speed = 100;
    if (speed < -100) speed = -100;

    // Going through 0 when reversing protects the H-bridge from a hard flip
    if ((speed > 0 && m->speed < 0) || (speed < 0 && m->speed > 0)) {
        pwm_set_gpio_level(m->en, 0);
        gpio_put(m->in1, 0);
        gpio_put(m->in2, 0);
        sleep_ms(50);
    }

    gpio_put(m->in1, speed > 0);
    gpio_put(m->in2, speed < 0);
    int duty = (speed < 0 ? -speed : speed) * (PWM_WRAP + 1) / 100;
    pwm_set_gpio_level(m->en, (uint16_t)duty);
    m->speed = (int8_t)speed;
}

void motor_stop(motor_t *m) {
    pwm_set_gpio_level(m->en, 0);
    gpio_put(m->in1, 0);
    gpio_put(m->in2, 0);
    m->speed = 0;
}

void motor_brake(motor_t *m) {
    gpio_put(m->in1, 1);
    gpio_put(m->in2, 1);
    pwm_set_gpio_level(m->en, PWM_WRAP + 1);   // full enable = both ends shorted
    m->speed = 0;
}

int8_t motor_get(const motor_t *m) {
    return m->speed;
}
