// Buzzer test: plays real tones with the buzzer driver (PWM), so it works
// with passive buzzers (need a tone) and active buzzers alike.
// Pattern: 2 kHz beep 0.5 s, pause, 1 kHz beep 0.5 s, pause, ready tone.
// Button on GP15 also plays a short beep, so both parts are tested.
//
// Wiring: GP14 (pin 19) -> resistor -> buzzer +, buzzer - -> GND.

#include <stdio.h>
#include "pico/stdlib.h"
#include "buzzer.h"
#include "button.h"

#define BUTTON_PIN 15

int main(void) {
    stdio_init_all();
    buzzer_init();
    button_t btn;
    button_init(&btn, BUTTON_PIN);

    uint32_t next = 0;
    int step = 0;
    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        button_update(&btn);
        if (button_pressed(&btn)) {
            printf("BUTTON -> beep\n");
            buzzer_beep(2500, 80);
        }
        if (now >= next) {
            switch (step) {
                case 0: printf("TONE 2000 Hz\n"); buzzer_beep(2000, 500); break;
                case 1: printf("TONE 1000 Hz\n"); buzzer_beep(1000, 500); break;
                case 2: printf("READY TONE\n");   buzzer_ready_tone();    break;
            }
            step = (step + 1) % 3;
            next = to_ms_since_boot(get_absolute_time()) + 1000;
        }
        sleep_ms(5);
    }
}
