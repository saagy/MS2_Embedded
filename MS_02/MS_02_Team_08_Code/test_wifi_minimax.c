// Wi-Fi Minimax test: laptop --(Wi-Fi)--> ESP-01S --(UART0)--> Pico
//
// The laptop sends one board per TCP connection to the ESP (port 333):
//   9 characters row by row ('.', 'O', 'X') + '\n', e.g. "O...X...O\n"
// The Pico runs Minimax and sends back one line, same as test_minimax_usb:
//   "MOVE <cell>", "O TURN", "O WINS", "X WINS", "DRAW", "ERROR BOARD", "ERROR LENGTH"
//
// Wiring (ESP-01S):  ESP TX -> GP1 (pin 2)   ESP RX -> GP0 (pin 1)
//                    ESP 3V3 -> pin 36       ESP GND -> GND
// The ESP must already know the hotspot (AT+CWJAP was done once; it is saved).
// Everything is also printed on USB serial (Serial Monitor, 115200).

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "minimax.h"

#define ESP_UART   uart0
#define ESP_TX_PIN 0
#define ESP_RX_PIN 1
#define ESP_BAUD   115200
#define TCP_PORT   "333"

// ---------------- small ESP helpers ----------------

static char resp[512];      // last response collected by esp_cmd
static unsigned resp_len;

static uint32_t now_ms(void) { return to_ms_since_boot(get_absolute_time()); }

// Send an AT command and collect the reply until `expect` (or ERROR/FAIL)
// shows up or the timeout ends. Returns true if `expect` was seen.
static bool esp_cmd(const char *cmd, const char *expect, uint32_t timeout_ms) {
    while (uart_is_readable(ESP_UART)) uart_getc(ESP_UART);   // drop old bytes
    resp_len = 0; resp[0] = '\0';
    if (cmd) {
        uart_puts(ESP_UART, cmd);
        uart_puts(ESP_UART, "\r\n");
    }
    uint32_t end = now_ms() + timeout_ms;
    while (now_ms() < end) {
        while (uart_is_readable(ESP_UART)) {
            char c = uart_getc(ESP_UART);
            if (resp_len < sizeof resp - 1) { resp[resp_len++] = c; resp[resp_len] = '\0'; }
        }
        if (strstr(resp, expect)) return true;
        if (strstr(resp, "ERROR") || strstr(resp, "FAIL")) return false;
    }
    return false;
}

// Send `data` to connection `id` (AT+CIPSEND, wait for '>', send, wait SEND OK)
static bool esp_send(int id, const char *data) {
    char cmd[32];
    snprintf(cmd, sizeof cmd, "AT+CIPSEND=%d,%u", id, (unsigned)strlen(data));
    if (!esp_cmd(cmd, ">", 2000)) return false;
    uart_puts(ESP_UART, data);
    return esp_cmd(NULL, "SEND OK", 3000);
}

static bool esp_setup(void) {
    sleep_ms(1000);                                   // ESP boot time
    for (int i = 0; i < 5 && !esp_cmd("AT", "OK", 1000); i++) printf("waiting for ESP...\n");
    if (!esp_cmd("AT", "OK", 1000)) { printf("ESP not answering - check wiring.\n"); return false; }
    esp_cmd("ATE0", "OK", 1000);                      // no echo = simpler replies
    esp_cmd("AT+CWMODE=1", "OK", 1000);

    // The ESP rejoins the saved hotspot by itself; wait until it has an IP
    printf("Waiting for Wi-Fi (hotspot on?) ...\n");
    char ip[24] = "";
    for (int i = 0; i < 30; i++) {
        if (esp_cmd("AT+CIFSR", "OK", 2000)) {
            char *p = strstr(resp, "STAIP,\"");
            if (p) {
                p += 7;
                char *q = strchr(p, '"');
                if (q && q - p < (int)sizeof ip) { memcpy(ip, p, q - p); ip[q - p] = '\0'; }
            }
            if (ip[0] && strcmp(ip, "0.0.0.0") != 0) break;
        }
        ip[0] = '\0';
        sleep_ms(1000);
    }
    if (!ip[0]) { printf("No Wi-Fi IP. Is the hotspot on? (AT+CWJAP once)\n"); return false; }

    esp_cmd("AT+CIPMUX=1", "OK", 1000);
    if (!esp_cmd("AT+CIPSERVER=1," TCP_PORT, "OK", 2000)) {
        printf("Could not start the server.\n"); return false;
    }
    printf("=== READY: ESP at %s, port %s ===\n", ip, TCP_PORT);
    printf("On the laptop:  python wifi_minimax_test.py %s\n", ip);
    return true;
}

// ---------------- Minimax answer (same text as test_minimax_usb) ----------------

static void answer_for(const char *line, unsigned len, char *out, size_t outsz) {
    while (len && (line[len - 1] == '\n' || line[len - 1] == '\r')) len--;
    if (len != 9) { snprintf(out, outsz, "ERROR LENGTH\n"); return; }
    char board[9];
    memcpy(board, line, 9);
    switch (ttt_status(board)) {
        case TTT_X_TURN: snprintf(out, outsz, "MOVE %d\n", minimax_best_move(board)); break;
        case TTT_O_TURN: snprintf(out, outsz, "O TURN\n");      break;
        case TTT_O_WINS: snprintf(out, outsz, "O WINS\n");      break;
        case TTT_X_WINS: snprintf(out, outsz, "X WINS\n");      break;
        case TTT_DRAW:   snprintf(out, outsz, "DRAW\n");        break;
        default:         snprintf(out, outsz, "ERROR BOARD\n"); break;
    }
}

// ---------------- main: wait for "+IPD,<id>,<len>:<data>" ----------------

int main(void) {
    stdio_init_all();
    uart_init(ESP_UART, ESP_BAUD);
    gpio_set_function(ESP_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(ESP_RX_PIN, GPIO_FUNC_UART);
    sleep_ms(2000);   // time to open the Serial Monitor
    printf("\n=== Wi-Fi Minimax test ===\n");

    bool ready = esp_setup();

    char hdr[32]; unsigned hlen = 0;       // collects "+IPD,0,10"
    char data[64]; unsigned dlen = 0, need = 0;
    int conn = -1;
    bool in_data = false;

    while (true) {
        if (!ready) {                      // retry setup every 5 s
            sleep_ms(5000);
            ready = esp_setup();
            continue;
        }
        if (!uart_is_readable(ESP_UART)) { sleep_ms(1); continue; }
        char c = uart_getc(ESP_UART);

        if (in_data) {                     // payload bytes after ':'
            if (dlen < sizeof data - 1) data[dlen++] = c;
            if (--need == 0) {
                in_data = false;
                data[dlen] = '\0';
                char reply[24];
                answer_for(data, dlen, reply, sizeof reply);
                printf("Board from laptop: %.*s  ->  %s", (int)(dlen > 9 ? 9 : dlen), data, reply);
                if (!esp_send(conn, reply)) printf("(sending the reply failed)\n");
                hlen = 0;
            }
            continue;
        }

        if (c == '\n' || c == '\r') { hlen = 0; continue; }
        if (hlen < sizeof hdr - 1) { hdr[hlen++] = c; hdr[hlen] = '\0'; }
        if (c == ':' && strncmp(hdr, "+IPD,", 5) == 0) {
            int id = 0, len = 0;
            if (sscanf(hdr, "+IPD,%d,%d:", &id, &len) == 2 && len > 0) {
                conn = id; need = (unsigned)len; dlen = 0; in_data = true;
            }
            hlen = 0;
        }
    }
}
