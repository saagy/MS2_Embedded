// Hardware test: ESP-01S (AI-Thinker, AT firmware) on UART0.
//
// Wiring:  ESP TX  -> GP1 (pin 2, UART0 RX)
//          ESP RX  -> GP0 (pin 1, UART0 TX)
//          ESP 3V3 -> 3V3 OUT (pin 36)   ESP GND -> GND
//
// What it does:
//  1. Sends "AT" to the ESP every 2 s until the ESP answers "OK",
//     then prints "ESP-01S OK" and asks for its firmware version (AT+GMR).
//  2. After that it is a bridge: whatever you type in the Serial Monitor
//     (line ending LF or CRLF) is sent to the ESP, and every reply from the
//     ESP is printed. Try:  AT   AT+GMR   AT+CWMODE=1   AT+CWLAP
//  The onboard LED is not used (works on Pico and Pico W).

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define ESP_UART     uart0
#define ESP_TX_PIN   0      // Pico TX -> ESP RX
#define ESP_RX_PIN   1      // Pico RX <- ESP TX
#define ESP_BAUD     115200 // AI-Thinker AT firmware default

static void esp_send(const char *s) {
    uart_puts(ESP_UART, s);
}

int main(void) {
    stdio_init_all();
    uart_init(ESP_UART, ESP_BAUD);
    gpio_set_function(ESP_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(ESP_RX_PIN, GPIO_FUNC_UART);

    sleep_ms(2000);   // time to open the Serial Monitor
    printf("\n=== ESP-01S test: sending AT every 2 s ===\n");

    bool found = false;
    char reply[64];
    unsigned rlen = 0;
    uint32_t next_at = 0, tries = 0;

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());

        // Keep poking the ESP until it answers
        if (!found && now >= next_at) {
            tries++;
            printf("> AT   (try %lu)\n", (unsigned long)tries);
            esp_send("AT\r\n");
            next_at = now + 2000;
            if (tries == 5) {
                printf("!! No answer yet. Check: ESP TX->GP1, ESP RX->GP0,\n"
                       "!! 3V3->pin 36, GND->GND (ESP blue LED should flash at power-up)\n");
            }
        }

        // Everything the ESP says -> Serial Monitor
        while (uart_is_readable(ESP_UART)) {
            char c = uart_getc(ESP_UART);
            putchar(c);
            if (!found) {
                if (c == '\n' || rlen == sizeof reply - 1) {
                    reply[rlen] = '\0';
                    if (strstr(reply, "OK")) {
                        found = true;
                        printf("\n=== ESP-01S OK - it answers! ===\n");
                        printf("> AT+GMR (firmware version)\n");
                        esp_send("AT+GMR\r\n");
                        printf("Now type AT commands in the Serial Monitor.\n");
                    }
                    rlen = 0;
                } else if (c != '\r') {
                    reply[rlen++] = c;
                }
            }
        }

        // Serial Monitor -> ESP (newline becomes CR LF, as AT commands need)
        int ch = getchar_timeout_us(0);
        if (ch != PICO_ERROR_TIMEOUT) {
            if (ch == '\n')      esp_send("\r\n");
            else if (ch != '\r') uart_putc_raw(ESP_UART, (char)ch);
        }
    }
}
