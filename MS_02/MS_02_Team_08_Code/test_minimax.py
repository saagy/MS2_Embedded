"""Compile the solver with host GCC and check it against unpruned Minimax."""

from collections import Counter
from functools import cache
from itertools import product
from pathlib import Path
import subprocess
import tempfile


LINES = ("012", "345", "678", "036", "147", "258", "048", "246")


def winner(board):
    for a, b, c in LINES:
        if board[int(a)] == board[int(b)] == board[int(c)] != ".":
            return board[int(a)]
    return None


def moves(board, piece):
    for cell, value in enumerate(board):
        if value == ".":
            yield cell, board[:cell] + piece + board[cell + 1:]


@cache
def score(board, turn):
    result = winner(board)
    if result:
        return (1 if result == "X" else -1) * (1 + board.count("."))
    if "." not in board:
        return 0
    other = "O" if turn == "X" else "X"
    choose = max if turn == "X" else min
    return choose(score(next_board, other) for _, next_board in moves(board, turn))


def collect(board, turn, positions):
    if board in positions:
        return
    result = winner(board)
    status = {"O": 3, "X": 4}[result] if result else (
        5 if "." not in board else 1 if turn == "O" else 2
    )
    positions[board] = status
    if status in (1, 2):
        for _, next_board in moves(board, turn):
            collect(next_board, "X" if turn == "O" else "O", positions)


def verify(executable):
    positions = {}
    collect(".........", "O", positions)
    assert len(positions) == 5478
    boards = ["".join(cells) for cells in product(".OX", repeat=9)]
    boards.append("?........")
    run = subprocess.run([str(executable)], input="\n".join(boards) + "\n",
                         text=True, capture_output=True, check=True)
    rows = run.stdout.splitlines()
    assert len(rows) == len(boards)
    robot_moves = {}
    selected = {}
    for board, row in zip(boards, rows):
        actual_status, cell = map(int, row.split())
        status = positions.get(board, 0)
        assert actual_status == status, board
        if status != 2:
            assert cell == -1, board
            continue
        assert 0 <= cell < 9 and board[cell] == ".", (board, cell)
        next_board = board[:cell] + "X" + board[cell + 1:]
        assert score(next_board, "O") == score(board, "X"), (board, cell)
        robot_moves[board] = next_board
        selected[board] = cell

    assert selected["O........"] == 4
    assert selected["....O...."] == 0
    assert selected["O...X...O"] == 1

    def play(board):
        status = positions[board]
        if status == 1:
            for _, next_board in moves(board, "O"):
                yield from play(next_board)
        elif status == 2:
            yield from play(robot_moves[board])
        else:
            yield status

    outcomes = Counter(play("........."))
    assert outcomes[3] == 0
    print(f"Checked all {3 ** 9} boards and {len(robot_moves)} optimal X moves.")
    print(f"Against this solver: {outcomes[4]} X wins, {outcomes[5]} draws, "
          f"{outcomes[3]} O wins.")


if __name__ == "__main__":
    source = Path(__file__).with_name("minimax.c")
    with tempfile.TemporaryDirectory() as directory:
        executable = Path(directory) / "test_minimax.exe"
        runner = Path(directory) / "runner.c"
        runner.write_text(r'''
#include <stdio.h>
#include "minimax.h"

int main(void) {
    char board[11];
    while (fgets(board, sizeof board, stdin))
        printf("%d %d\n", ttt_status(board), minimax_best_move(board));
    return 0;
}
''', encoding="ascii")
        subprocess.run(["gcc", "-std=c99", "-Wall", "-Wextra", "-Werror",
                        "-I", str(source.parent), str(source), str(runner),
                        "-o", str(executable)], check=True)
        verify(executable)
