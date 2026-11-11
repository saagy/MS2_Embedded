#include "buzzer.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

static uint slice, channel;

void buzzer_init(void) {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    slice   = pwm_gpio_to_slice_num(BUZZER_PIN);
    channel = pwm_gpio_to_channel(BUZZER_PIN);

    // Divide the system clock down to a 1 MHz PWM tick
    pwm_set_clkdiv(slice, (float)clock_get_hz(clk_sys) / 1000000.0f);
    pwm_set_chan_level(slice, channel, 0);
    pwm_set_enabled(slice, true);
}

void buzzer_tone(uint16_t freq_hz) {
    if (freq_hz < 20) { buzzer_off(); return; }
    uint32_t wrap = 1000000u / freq_hz - 1;    // 1 MHz / freq
    pwm_set_wrap(slice, (uint16_t)wrap);
    pwm_set_chan_level(slice, channel, (uint16_t)(wrap / 2)); // 50% duty
}

void buzzer_off(void) {
    pwm_set_chan_level(slice, channel, 0);
}

void buzzer_beep(uint16_t freq_hz, uint16_t ms) {
    buzzer_tone(freq_hz);
    sleep_ms(ms);
    buzzer_off();
}

void buzzer_ready_tone(void) {
    buzzer_beep(1000, 100);
    sleep_ms(40);
    buzzer_beep(1500, 150);
}

void buzzer_fail_tones(void) {
    for (int i = 0; i < 5; i++) {
        buzzer_beep(400, 80);
        sleep_ms(60);
    }
}
