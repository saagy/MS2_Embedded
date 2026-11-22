#include "minimax.h"

#include <stdbool.h>
#include <string.h>

static const unsigned char winning_lines[][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
    {0, 4, 8}, {2, 4, 6}
};
static const unsigned char move_order[] = {4, 0, 2, 6, 8, 1, 3, 5, 7};

static bool has_won(const char board[9], char piece) {
    for (unsigned i = 0; i < sizeof winning_lines / sizeof winning_lines[0]; ++i) {
        const unsigned char *line = winning_lines[i];
        if (board[line[0]] == piece && board[line[1]] == piece &&
            board[line[2]] == piece)
            return true;
    }
    return false;
}

ttt_status_t ttt_status(const char board[9]) {
    if (!board)
        return TTT_INVALID;

    int o_count = 0;
    int x_count = 0;
    for (int i = 0; i < 9; ++i) {
        switch (board[i]) {
            case 'O': ++o_count; break;
            case 'X': ++x_count; break;
            case '.': break;
            default: return TTT_INVALID;
        }
    }
    if (o_count != x_count && o_count != x_count + 1)
        return TTT_INVALID;

    bool o_won = has_won(board, 'O');
    bool x_won = has_won(board, 'X');
    if (o_won)
        return !x_won && o_count == x_count + 1 ? TTT_O_WINS : TTT_INVALID;
    if (x_won)
        return o_count == x_count ? TTT_X_WINS : TTT_INVALID;
    if (o_count + x_count == 9)
        return TTT_DRAW;
    return o_count == x_count ? TTT_O_TURN : TTT_X_TURN;
}

static int search(char board[9], char turn, int remaining, int alpha, int beta) {
    char previous = turn == 'X' ? 'O' : 'X';
    if (has_won(board, previous))
        return previous == 'X' ? 1 + remaining : -1 - remaining;
    if (remaining == 0)
        return 0;

    int best = turn == 'X' ? -10 : 10;
    for (unsigned i = 0; i < sizeof move_order; ++i) {
        int cell = move_order[i];
        if (board[cell] != '.')
            continue;
        board[cell] = turn;
        int score = search(board, previous, remaining - 1, alpha, beta);
        board[cell] = '.';

        if (turn == 'X') {
            if (score > best)
                best = score;
            if (best > alpha)
                alpha = best;
        } else {
            if (score < best)
                best = score;
            if (best < beta)
                beta = best;
        }
        if (alpha >= beta)
            break;
    }
    return best;
}

int minimax_best_move(const char board[9]) {
    if (ttt_status(board) != TTT_X_TURN)
        return -1;

    char working[9];
    memcpy(working, board, sizeof working);
    int remaining = 0;
    for (int i = 0; i < 9; ++i)
        remaining += working[i] == '.';

    int best_cell = -1;
    int best_score = -10;
    for (unsigned i = 0; i < sizeof move_order; ++i) {
        int cell = move_order[i];
        if (working[cell] != '.')
            continue;
        working[cell] = 'X';
        int score = search(working, 'O', remaining - 1, best_score, 10);
        working[cell] = '.';
        if (score > best_score) {
            best_score = score;
            best_cell = cell;
        }
    }
    return best_cell;
}
