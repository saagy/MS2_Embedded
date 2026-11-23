// Hardware test: SG90 micro servo on GP28.
//
// Press the button to START, press again to STOP (servo goes limp).
// While running it repeats, holding each position for 3 seconds:
//   one end (0 deg) -> other end (180 deg) -> back ... (a 180 deg swing)
// Serial prints every position, e.g. "SERVO +90 deg".

#include <stdio.h>
#include "pico/stdlib.h"
#include "servo.h"
#include "button.h"

#define BUTTON_PIN 15
#define STEP_MS    3000

static const int steps[] = {-90, 90};   // 0 deg <-> 180 deg (full range)
#define NSTEPS (sizeof steps / sizeof steps[0])

int main(void) {
    stdio_init_all();
    servo_init(SERVO_PIN);
    servo_off();
    button_t btn;
    button_init(&btn, BUTTON_PIN);

    bool running = false;
    unsigned i = 0;
    uint32_t next = 0, last_hint = 0;

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        button_update(&btn);

        if (button_pressed(&btn)) {
            running = !running;
            i = 0;
            next = now;
            if (!running) servo_off();
            printf(running ? "=== SERVO TEST STARTED ===\n"
                           : "=== SERVO STOPPED ===\n");
        }

        if (!running) {
            if (now - last_hint >= 3000) {
                printf("Servo OFF. Press the button to start.\n");
                last_hint = now;
            }
        } else if (now >= next) {
            servo_write_signed(steps[i]);
            printf("SERVO %3d deg\n", steps[i] + 90);
            i = (i + 1) % NSTEPS;
            next = now + STEP_MS;
        }
        sleep_ms(5);
    }
}
