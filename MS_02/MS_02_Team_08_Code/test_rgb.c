// Phase 1 hardware test: RGB LED only (no LCD, button or pots needed).
// Cycles red -> green -> blue -> yellow -> off and prints each colour
// on USB serial, so you can check the LED matches the text.
//
// Wiring: GP16 -> 330R -> red leg, GP17 -> 330R -> green leg,
//         GP18 -> 330R -> blue leg, long (common) leg -> GND pin 23.
// If nothing lights, the LED is common anode: move the long leg to
// 3V3 (pin 36) and set RGB_COMMON_ANODE to 1 in rgb_led.h.

#include <stdio.h>
#include "pico/stdlib.h"
#include "rgb_led.h"

static void show(const char *name, uint8_t r, uint8_t g, uint8_t b) {
    printf("%s\n", name);
    rgb_set(r, g, b);
    sleep_ms(1000);
}

int main(void) {
    stdio_init_all();
    rgb_init();

    while (true) {
        show("RED",    255,   0,   0);
        show("GREEN",    0, 255,   0);
        show("BLUE",     0,   0, 255);
        show("YELLOW", 255, 255,   0);
        show("OFF",      0,   0,   0);
    }
}
