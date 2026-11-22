// Wiring scan: every free GPIO (including the buzzer pin GP14) is set as an
// input with a pull-up. A pin wired through the buzzer coil (+ resistor) to
// GND reads LOW, so this shows which pin the buzzer is really connected to.
// Prints the list of LOW pins once per second.

#include <stdio.h>
#include "pico/stdlib.h"

static const uint pins[] = { 0, 1, 2, 3, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                             19, 20, 21, 22, 28 };
#define N (sizeof pins / sizeof pins[0])

int main(void) {
    stdio_init_all();
    for (uint i = 0; i < N; i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_up(pins[i]);
    }
    while (true) {
        printf("LOW pins:");
        bool any = false;
        for (uint i = 0; i < N; i++)
            if (!gpio_get(pins[i])) { printf(" GP%u", pins[i]); any = true; }
        printf(any ? "\n" : " none\n");
        sleep_ms(1000);
    }
}
