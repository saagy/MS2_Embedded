// Diagnostic: button + buzzer wiring check (no LCD needed).
//
// Buzzer: GP14 is switched fully ON for 1 s, OFF for 1 s (plain on/off,
//         the best signal for an active buzzer). Serial says BUZZER ON/OFF.
// Button: every free GPIO is read with a pull-up. Any pin pulled LOW is
//         reported, so we can see which pin the button is really on.
//         The LED turns blue while any pin is pulled low.

#include <stdio.h>
#include "pico/stdlib.h"
#include "rgb_led.h"

#define BUZZER 14
#define BUTTON 15

// Free GPIOs to watch (skips LED 16-18, ADC 26-27, buzzer 14, Pico W internals)
static const uint watch[] = { 0, 1, 2, 3, 6, 7, 8, 9, 10, 11, 12, 13, 15,
                              19, 20, 21, 22, 28 };
#define NWATCH (sizeof watch / sizeof watch[0])

int main(void) {
    stdio_init_all();
    rgb_init();

    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
    gpio_put(BUZZER, 0);

    for (uint i = 0; i < NWATCH; i++) {
        gpio_init(watch[i]);
        gpio_set_dir(watch[i], GPIO_IN);
        gpio_pull_up(watch[i]);
    }
    sleep_ms(50);

    bool was_low[NWATCH] = { false };
    uint32_t presses = 0;
    bool buzz = false;
    uint32_t next_buzz = 0;

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());

        if (now >= next_buzz) {
            buzz = !buzz;
            gpio_put(BUZZER, buzz);
            printf(buzz ? "BUZZER ON\n" : "BUZZER OFF\n");
            next_buzz = now + 1000;
        }

        bool any_low = false;
        for (uint i = 0; i < NWATCH; i++) {
            bool low = !gpio_get(watch[i]);
            if (low) any_low = true;
            if (low != was_low[i]) {
                if (low && watch[i] == BUTTON) presses++;
                printf("GP%u %s%s\n", watch[i], low ? "LOW (pressed/connected)" : "released",
                       watch[i] == BUTTON && low ? "  <- button pin OK" : "");
                was_low[i] = low;
            }
        }
        rgb_set(0, 0, any_low ? 255 : 0);

        sleep_ms(10);
    }
}
