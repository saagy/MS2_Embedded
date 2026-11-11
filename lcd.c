
#include "lcd.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

static uint8_t lcd_addr;
#define LCD_I2C i2c0
#define RS 0x01
#define EN 0x04
#define BL 0x08

static void write_expander(uint8_t data) {
    uint8_t value = data | BL;
    i2c_write_blocking(LCD_I2C, lcd_addr,
                       &value, 1, false);
}

static void send_nibble(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0xF0) | mode;
    write_expander(data);
    sleep_us(5);
    write_expander(data | EN);
    sleep_us(10);              // EN pulse width
    write_expander(data & ~EN);
    sleep_us(100);             // command settle time
}

static void lcd_send(uint8_t data, uint8_t mode) {
    send_nibble(data & 0xF0, mode);
    send_nibble((data << 4) & 0xF0, mode);
}

static void lcd_cmd(uint8_t cmd) {
    lcd_send(cmd, 0);
    if (cmd == 0x01 || cmd == 0x02)
        sleep_ms(2);
}

void lcd_init(uint8_t address) {
    lcd_addr = address;
    write_expander(0x00);      // all lines low, backlight on
    sleep_ms(200);             // let the LCD power up fully

    send_nibble(0x30, 0);
    sleep_ms(10);
    send_nibble(0x30, 0);
    sleep_ms(5);
    send_nibble(0x30, 0);
    sleep_ms(5);
    send_nibble(0x20, 0);      // switch to 4-bit mode
    sleep_ms(5);

    lcd_cmd(0x28); // 4-bit, 2 lines
    lcd_cmd(0x08); // Display off
    lcd_clear();
    lcd_cmd(0x06); // Entry mode
    lcd_cmd(0x0C); // Display on
}

void lcd_clear(void) {
    lcd_cmd(0x01);
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
    if (row > 1 || col > 15) return;
    lcd_cmd(0x80 | (row ? 0x40 : 0x00) | col);
}

void lcd_print(const char *text) {
    while (*text)
        lcd_send((uint8_t)*text++, RS);
}

