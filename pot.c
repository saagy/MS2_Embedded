#include "pot.h"
#include "hardware/adc.h"

#define POT_SAMPLES 8   // average a few readings to remove jitter

static bool adc_ready = false;

void pot_init(uint8_t gpio) {
    if (!adc_ready) {
        adc_init();
        adc_ready = true;
    }
    adc_gpio_init(gpio);   // disables digital functions on the pin
}

uint16_t pot_read(uint8_t gpio) {
    adc_select_input(gpio - 26);   // GP26 -> 0, GP27 -> 1, GP28 -> 2

    uint32_t sum = 0;
    for (int i = 0; i < POT_SAMPLES; i++)
        sum += adc_read();         // 12-bit: 0..4095

    return (uint16_t)(sum / POT_SAMPLES);
}

uint8_t pot_percent(uint8_t gpio) {
    return (uint8_t)((pot_read(gpio) * 100u) / 4095u);
}
