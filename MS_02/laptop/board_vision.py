"""Tic-tac-toe board reader (Team 08).

How it works
1. The camera looks straight down; the board is lined up with a 3x3 grid
   in a centered square (same grid as webcam_test.py / link_test.py).
2. Calibrate once with the EMPTY board: each cell's picture is stored.
3. For every new frame, each cell is compared with its empty picture.
   A big difference = a piece is there.
4. The changed area (the piece) is outlined and its shape decides:
     O : ring/round shape -> has a hole in the middle and is circular
     X : cross shape      -> thin arms, so it fills little of its outline
   If a shape is unclear the cell is marked '?'.

Board format: list of 9 chars, row by row, 'X', 'O', '.' (empty) or '?'.
"""
import cv2
import numpy as np

# ---- Tunable settings -------------------------------------------------------
GRID_FRACTION = 0.9    # grid square = 90 % of the shorter image side
CELL_MARGIN = 0.15     # ignore 15 % at each cell edge (board lines, shadows)
DIFF_THRESHOLD = 35    # grey-level change that counts as "piece" pixels
MIN_FILL = 0.06        # piece must cover at least 6 % of the cell
O_MIN_CIRCULARITY = 0.55
X_MAX_SOLIDITY = 0.70


def grid_square(frame):
    """Return (x0, y0, side) of the centered square the grid is drawn in."""
    h, w = frame.shape[:2]
    side = int(min(w, h) * GRID_FRACTION)
    return (w - side) // 2, (h - side) // 2, side


def cell_boxes(frame):
    """Inner box (x, y, w, h) of each of the 9 cells, row by row."""
    x0, y0, side = grid_square(frame)
    c = side // 3
    m = int(c * CELL_MARGIN)
    return [(x0 + col * c + m, y0 + row * c + m, c - 2 * m, c - 2 * m)
            for row in range(3) for col in range(3)]


def draw_grid(frame, colour=(0, 255, 0)):
    x0, y0, side = grid_square(frame)
    for i in range(4):
        x = x0 + i * side // 3
        y = y0 + i * side // 3
        cv2.line(frame, (x, y0), (x, y0 + side), colour, 2)
        cv2.line(frame, (x0, y), (x0 + side, y), colour, 2)


def _prep(img):
    grey = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY) if img.ndim == 3 else img
    return cv2.GaussianBlur(grey, (5, 5), 0)


def calibrate(frame):
    """Store the empty board: returns one grey picture per cell."""
    return [_prep(frame[y:y + h, x:x + w]) for x, y, w, h in cell_boxes(frame)]


def classify_cell(cell, empty):
    """Return ('X' | 'O' | '.' | '?', info dict) for one cell."""
    diff = cv2.absdiff(_prep(cell), empty)
    _, mask = cv2.threshold(diff, DIFF_THRESHOLD, 255, cv2.THRESH_BINARY)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, np.ones((3, 3), np.uint8))
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, np.ones((5, 5), np.uint8))

    fill = cv2.countNonZero(mask) / mask.size
    info = {"fill": fill, "circ": 0.0, "solid": 0.0, "hole": False, "mask": mask}
    if fill < MIN_FILL:
        return ".", info

    contours, hier = cv2.findContours(mask, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_SIMPLE)
    if not contours:
        return ".", info
    # biggest outer contour = the piece
    outer = [i for i in range(len(contours)) if hier[0][i][3] == -1]
    best = max(outer, key=lambda i: cv2.contourArea(contours[i]))
    cnt = contours[best]
    area = cv2.contourArea(cnt)
    perim = cv2.arcLength(cnt, True)
    hull_area = cv2.contourArea(cv2.convexHull(cnt))
    circ = 4 * np.pi * area / (perim * perim) if perim else 0.0
    solid = area / hull_area if hull_area else 0.0
    # a hole = a child contour that is reasonably large
    child = hier[0][best][2]
    hole = False
    while child != -1:
        if cv2.contourArea(contours[child]) > 0.05 * area:
            hole = True
            break
        child = hier[0][child][0]

    info.update(circ=circ, solid=solid, hole=hole)
    if hole and circ >= O_MIN_CIRCULARITY:
        return "O", info
    if solid <= X_MAX_SOLIDITY and not hole:
        return "X", info
    if circ >= 0.80:          # solid round disc: treat as O
        return "O", info
    return "?", info


def read_board(frame, empty_cells):
    """Classify all 9 cells. Returns (board list, list of info dicts)."""
    board, infos = [], []
    for (x, y, w, h), empty in zip(cell_boxes(frame), empty_cells):
        mark, info = classify_cell(frame[y:y + h, x:x + w], empty)
        board.append(mark)
        infos.append(info)
    return board, infos


def board_text(board):
    return "\n".join(" ".join(board[r * 3:r * 3 + 3]) for r in range(3))
