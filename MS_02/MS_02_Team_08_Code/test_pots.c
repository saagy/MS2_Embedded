// Phase 3 hardware test: 2 potentiometers + status RGB LED (no LCD needed).
//
// Required behaviour:
//   - P1 turned clockwise  -> mode POWERED ON, LED red
//   - P2 turned clockwise (while P1 is on) -> mode ENGAGED, LED green
//   - P2 turned while P1 is not -> LED stays off
//
// Wiring: both pots: outer legs -> 3V3 (pin 36) and AGND (pin 33).
//         P1 middle leg -> GP26 (pin 31), P2 middle leg -> GP27 (pin 32).
// If turning clockwise makes the % go DOWN, swap that pot's outer legs.

#include <stdio.h>
#include "pico/stdlib.h"
#include "pot.h"
#include "rgb_led.h"

#define POT1_PIN 26
#define POT2_PIN 27

// Hysteresis so the mode doesn't flicker when a knob sits near the middle
#define ON_ABOVE  60   // % turned clockwise past this -> ON
#define OFF_BELOW 40   // % turned back below this     -> OFF

typedef enum { MODE_OFF, MODE_POWERED, MODE_ENGAGED } sys_mode_t;

static const char *mode_name[] = { "OFF", "POWERED ON", "ENGAGED" };

static bool switch_state(bool current, uint8_t percent) {
    if (percent >= ON_ABOVE)  return true;
    if (percent <= OFF_BELOW) return false;
    return current;            // in the dead band: keep previous state
}

int main(void) {
    stdio_init_all();
    pot_init(POT1_PIN);
    pot_init(POT2_PIN);
    rgb_init();

    bool p1_on = false, p2_on = false;
    sys_mode_t last_mode = MODE_OFF;
    int tick = 0;

    while (true) {
        uint8_t p1 = pot_percent(POT1_PIN);
        uint8_t p2 = pot_percent(POT2_PIN);

        p1_on = switch_state(p1_on, p1);
        p2_on = switch_state(p2_on, p2);

        // P2 only counts if P1 is already on
        sys_mode_t mode = !p1_on ? MODE_OFF
                        : (p2_on ? MODE_ENGAGED : MODE_POWERED);

        switch (mode) {
            case MODE_OFF:     rgb_off();          break;
            case MODE_POWERED: rgb_set(255, 0, 0); break;  // red
            case MODE_ENGAGED: rgb_set(0, 255, 0); break;  // green
        }

        if (mode != last_mode) {
            printf(">>> MODE: %s\n", mode_name[mode]);
            last_mode = mode;
        }
        if (++tick >= 10) {    // status line every ~0.5 s
            tick = 0;
            printf("P1=%3u%% (%s)  P2=%3u%% (%s)  MODE=%s\n",
                   p1, p1_on ? "on " : "off", p2, p2_on ? "on " : "off",
                   mode_name[mode]);
        }
        sleep_ms(50);
    }
}
