#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>

// Push button driver. Wire one leg to the GPIO and the other to GND;
// the internal pull-up is used, so pressed = LOW. Debounced in software.

typedef struct {
    uint8_t  pin;
    bool     stable;        // debounced state (true = pressed)
    bool     last_raw;
    bool     prev_stable;   // for press (edge) detection
    uint32_t last_change_ms;
} button_t;

void button_init(button_t *b, uint8_t pin);
void button_update(button_t *b);        // call often (every loop)
bool button_is_down(const button_t *b); // debounced level
bool button_pressed(button_t *b);       // true once per press (edge)

#endif
