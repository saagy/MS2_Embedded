#include "button.h"
#include "pico/stdlib.h"

#define DEBOUNCE_MS 30

static bool read_raw(uint8_t pin) {
    return !gpio_get(pin);              // pull-up: LOW means pressed
}

void button_init(button_t *b, uint8_t pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
    sleep_ms(1);

    b->pin = pin;
    b->stable = b->last_raw = b->prev_stable = read_raw(pin);
    b->last_change_ms = to_ms_since_boot(get_absolute_time());
}

void button_update(button_t *b) {
    bool raw = read_raw(b->pin);
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (raw != b->last_raw) {           // input moved: restart timer
        b->last_raw = raw;
        b->last_change_ms = now;
    } else if (raw != b->stable && now - b->last_change_ms >= DEBOUNCE_MS) {
        b->stable = raw;                // held steady long enough
    }
}

bool button_is_down(const button_t *b) {
    return b->stable;
}

bool button_pressed(button_t *b) {
    bool fired = b->stable && !b->prev_stable;
    b->prev_stable = b->stable;
    return fired;
}
