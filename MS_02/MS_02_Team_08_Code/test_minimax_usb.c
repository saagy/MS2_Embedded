#include <stdio.h>

#include "pico/stdlib.h"
#include "minimax.h"

static void report_board(const char board[9]) {
    switch (ttt_status(board)) {
        case TTT_X_TURN: printf("MOVE %d\n", minimax_best_move(board)); break;
        case TTT_O_TURN: puts("O TURN"); break;
        case TTT_O_WINS: puts("O WINS"); break;
        case TTT_X_WINS: puts("X WINS"); break;
        case TTT_DRAW: puts("DRAW"); break;
        case TTT_INVALID: puts("ERROR BOARD"); break;
    }
}

int main(void) {
    stdio_init_all();
    char board[9];
    unsigned length = 0;

    while (true) {
        int ch = getchar_timeout_us(10000);
        if (ch == PICO_ERROR_TIMEOUT || ch == '\r')
            continue;
        if (ch == '\n') {
            if (length == sizeof board)
                report_board(board);
            else
                puts("ERROR LENGTH");
            length = 0;
        } else if (length < sizeof board) {
            board[length++] = (char)ch;
        } else {
            length = sizeof board + 1;
        }
    }
}
