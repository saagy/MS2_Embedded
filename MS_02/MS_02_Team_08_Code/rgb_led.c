#include "rgb_led.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

static void setup_pin(uint8_t pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice, 255);          // 8-bit brightness
    pwm_set_enabled(slice, true);
}

static void write_pin(uint8_t pin, uint8_t level) {
#if RGB_COMMON_ANODE
    level = 255 - level;               // common anode is active-low
#endif
    pwm_set_gpio_level(pin, level);
}

void rgb_init(void) {
    setup_pin(RGB_PIN_R);
    setup_pin(RGB_PIN_G);
    setup_pin(RGB_PIN_B);
    rgb_off();
}

void rgb_set(uint8_t r, uint8_t g, uint8_t b) {
    write_pin(RGB_PIN_R, r);
    write_pin(RGB_PIN_G, g);
    write_pin(RGB_PIN_B, b);
}

void rgb_off(void) {
    rgb_set(0, 0, 0);
}
