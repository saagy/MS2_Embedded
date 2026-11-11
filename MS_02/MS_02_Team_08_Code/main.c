#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lcd.h"
#include "pot.h"
#include "rgb_led.h"
#include "button.h"
#include "buzzer.h"

#define POT1_PIN   26 // GP26, pin 31  -> "Powered On"
#define POT2_PIN   27 // GP27, pin 32  -> "Engaged"
#define BUTTON_PIN 14 // GP14, pin 19  -> "Confirm Move"

// Hysteresis so the mode doesn't flicker when a knob sits at the middle
#define ON_ABOVE  60  // %  turned clockwise past this -> ON
#define OFF_BELOW 40  // %  turned back below this     -> OFF

typedef enum { MODE_OFF, MODE_POWERED, MODE_ENGAGED } sys_mode_t;

// Print a full 16-char line (pads with spaces, so no lcd_clear flicker)
static void lcd_line(uint8_t row, const char *text) {
    char buf[17];
    snprintf(buf, sizeof buf, "%-16s", text);
    lcd_set_cursor(row, 0);
    lcd_print(buf);
}

static bool switch_state(bool current, uint8_t percent) {
    if (percent >= ON_ABOVE)  return true;
    if (percent <= OFF_BELOW) return false;
    return current;           // in the dead band: keep previous state
}

static uint8_t find_lcd(void) {
    for (int addr = 0x20; addr <= 0x3F; addr++) {
        uint8_t data;
        if (i2c_read_blocking(i2c0, addr, &data, 1, false) == 1)
            return addr;
    }
    return 0;
}

int main(void) {
    stdio_init_all();

    // LCD on I2C0: SDA = GP4 (pin 6), SCL = GP5 (pin 7)
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);
    sleep_ms(500);

    uint8_t addr = find_lcd();
    if (!addr) {
        while (true) sleep_ms(1000);   // no LCD found
    }
    lcd_init(addr);

    pot_init(POT1_PIN);
    pot_init(POT2_PIN);
    rgb_init();
    buzzer_init();

    button_t btn;
    button_init(&btn, BUTTON_PIN);

    lcd_line(0, "SENSOR TEST");
    lcd_line(1, "Knobs + button");
    buzzer_ready_tone();               // startup beep = buzzer works
    sleep_ms(1500);

    bool p1_on = false, p2_on = false;
    unsigned presses = 0;
    char text[17];

    while (true) {
        button_update(&btn);

        uint8_t p1 = pot_percent(POT1_PIN);
        uint8_t p2 = pot_percent(POT2_PIN);

        p1_on = switch_state(p1_on, p1);
        p2_on = switch_state(p2_on, p2);

        // Project rule: P2 only counts if P1 is already on
        sys_mode_t mode = !p1_on ? MODE_OFF
                    : (p2_on ? MODE_ENGAGED : MODE_POWERED);

        switch (mode) {
            case MODE_OFF:      rgb_off();           break;
            case MODE_POWERED:  rgb_set(255, 0, 0);  break; // red
            case MODE_ENGAGED:  rgb_set(0, 255, 0);  break; // green
        }

        // Button test: each press beeps; in ENGAGED it plays the
        // "game start" tone, otherwise a short click.
        if (button_pressed(&btn)) {
            presses++;
            snprintf(text, sizeof text, "BUTTON #%u", presses);
            lcd_line(1, text);
            if (mode == MODE_ENGAGED) buzzer_ready_tone();
            else                      buzzer_beep(2000, 60);
            sleep_ms(400);             // keep the message visible
            continue;
        }

        snprintf(text, sizeof text, "P1:%3u%% P2:%3u%%", p1, p2);
        lcd_line(0, text);
        lcd_line(1, button_is_down(&btn) ? "BUTTON: PRESSED"
                  : mode == MODE_OFF     ? "MODE: OFF"
                  : mode == MODE_POWERED ? "MODE: POWERED ON"
                                         : "MODE: ENGAGED");

        sleep_ms(20);
    }
}
