// Phase 8 test: laptop <-> Pico serial link (webcam runs on the laptop).
//
// Laptop -> Pico (one character each, over USB serial):
//   'r' = LED red, 'g' = LED green, 'b' = LED blue, 'o' = LED off,
//   'p' = ping (Pico answers "PONG")
// Pico -> laptop:
//   "ACK <colour>" after each LED command
//   "BTN"          when the push button is pressed
//   "READY"        every 2 s until the laptop sends something

#include <stdio.h>
#include "pico/stdlib.h"
#include "rgb_led.h"
#include "button.h"

#define BUTTON_PIN 15

int main(void) {
    stdio_init_all();
    rgb_init();
    button_t btn;
    button_init(&btn, BUTTON_PIN);

    bool heard_laptop = false;
    uint32_t last_ready = 0;

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        button_update(&btn);

        int c = getchar_timeout_us(0);          // non-blocking read
        if (c != PICO_ERROR_TIMEOUT) {
            heard_laptop = true;
            switch (c) {
                case 'r': rgb_set(255, 0, 0); printf("ACK RED\n");   break;
                case 'g': rgb_set(0, 255, 0); printf("ACK GREEN\n"); break;
                case 'b': rgb_set(0, 0, 255); printf("ACK BLUE\n");  break;
                case 'o': rgb_off();          printf("ACK OFF\n");   break;
                case 'p':                     printf("PONG\n");      break;
                default: break;                 // ignore newlines etc.
            }
        }

        if (button_pressed(&btn)) printf("BTN\n");

        if (!heard_laptop && now - last_ready >= 2000) {
            printf("READY\n");
            last_ready = now;
        }
        sleep_ms(2);
    }
}
