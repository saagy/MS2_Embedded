#ifndef MINIMAX_H
#define MINIMAX_H

typedef enum {
    TTT_INVALID,
    TTT_O_TURN,
    TTT_X_TURN,
    TTT_O_WINS,
    TTT_X_WINS,
    TTT_DRAW
} ttt_status_t;

// Nine row-major cells: '.', 'O', or 'X'. O moves first.
ttt_status_t ttt_status(const char board[9]);

// Return the best X cell (0–8), or -1 unless it is X's turn.
int minimax_best_move(const char board[9]);

#endif
