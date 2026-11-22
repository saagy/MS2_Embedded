// Phase 5/6 hardware test: 12 V DC motor(s) through the L298N.
//
// SAFETY: the motors stay OFF until you press the button.
//         Press again at any time = immediate stop.
//
// Sequence (repeats while enabled), for Motor X then Motor Y:
//   forward 0 -> 100 % in 10 % steps, stop, reverse 0 -> -100 %, stop.
// LED: off = disabled, green = forward, blue = reverse, red = stopped/pause.
// Serial prints every step, e.g. "X  +40%".

#include <stdio.h>
#include "pico/stdlib.h"
#include "motor.h"
#include "button.h"
#include "rgb_led.h"

#define BUTTON_PIN 15
#define STEP_MS    500

// One entry per step of the test: which motor, which speed
typedef struct { char name; int speed; } step_t;

static step_t seq[64];
static int nseq = 0;

static void add_ramp(char name) {
    for (int s = 10; s <= 100; s += 10) seq[nseq++] = (step_t){ name, s };
    seq[nseq++] = (step_t){ name, 0 };
    for (int s = -10; s >= -100; s -= 10) seq[nseq++] = (step_t){ name, s };
    seq[nseq++] = (step_t){ name, 0 };
}

int main(void) {
    stdio_init_all();
    motor_t mx, my;
    motor_init(&mx, MOTOR_X_EN, MOTOR_X_IN1, MOTOR_X_IN2);
    motor_init(&my, MOTOR_Y_EN, MOTOR_Y_IN1, MOTOR_Y_IN2);
    rgb_init();
    button_t btn;
    button_init(&btn, BUTTON_PIN);

    add_ramp('X');
    add_ramp('Y');

    bool enabled = false;
    int i = 0;
    uint32_t next = 0;
    uint32_t last_hint = 0;

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        button_update(&btn);

        if (button_pressed(&btn)) {
            enabled = !enabled;
            motor_stop(&mx);
            motor_stop(&my);
            i = 0;
            next = now;
            printf(enabled ? "=== ENABLED: motor test starting ===\n"
                           : "=== STOPPED by button ===\n");
        }

        if (!enabled) {
            rgb_off();
            if (now - last_hint >= 3000) {
                printf("Motors OFF. Press the button to start.\n");
                last_hint = now;
            }
        } else if (now >= next) {
            step_t st = seq[i];
            motor_t *m = (st.name == 'X') ? &mx : &my;
            motor_t *other = (st.name == 'X') ? &my : &mx;
            motor_stop(other);
            motor_set(m, st.speed);

            if (st.speed > 0)      rgb_set(0, 255, 0);
            else if (st.speed < 0) rgb_set(0, 0, 255);
            else                   rgb_set(255, 0, 0);

            printf("%c  %+4d%%\n", st.name, st.speed);
            next = now + (st.speed == 0 ? 1500 : STEP_MS);
            i = (i + 1) % nseq;
        }
        sleep_ms(5);
    }
}
