"""Live tic-tac-toe board reader from the overhead USB webcam (Team 08).

Usage:  python board_detect.py [camera_index] [--port COM5] [--wifi 172.20.10.2]
        Camera defaults to 1. --port uses test_minimax_usb firmware (USB cable),
        --wifi uses test_wifi_minimax firmware (board sent over Wi-Fi to the ESP-01S).

Steps:
  1. Line the EMPTY board up with the green 3x3 grid.
  2. Click the video window and press  c  to calibrate (board must be empty).
  3. Place X and O pieces: each cell shows X, O, . (empty) or ? (unsure).
     The board is printed in the terminal whenever it changes and holds
     still for half a second.
Keys:  c = calibrate (empty board)   d = debug numbers on/off
       s = save snapshot            q = quit
       m = ask the Pico for a move from the current stable board (--port or --wifi)
"""
import argparse
import sys
import cv2
import board_vision as bv

STABLE_FRAMES = 15          # board must stay the same this many frames

parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
parser.add_argument("camera_index", nargs="?", type=int, default=1)
parser.add_argument("--port", help="USB serial port for test_minimax_usb")
parser.add_argument("--wifi", metavar="ESP_IP", help="ESP-01S IP for test_wifi_minimax")
args = parser.parse_args()

solver = None
if args.port:
    import serial
    solver = serial.Serial(args.port, 115200, timeout=1)
if args.wifi:
    import wifi_minimax_test as wifi   # ask(board, ip) -> "MOVE 4" etc.

cap = cv2.VideoCapture(args.camera_index, cv2.CAP_DSHOW)
if not cap.isOpened():
    sys.exit(f"Camera {args.camera_index} not found. Run camera_check.py.")
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

empty_cells = None
debug = False
last_board, same_count, printed = None, 0, None
# Marker colours (OpenCV uses B, G, R): X = blue, O = red
COL = {"X": (255, 0, 0), "O": (0, 0, 255), "?": (0, 200, 255), ".": (160, 160, 160)}

print("Line the EMPTY board up with the grid, click the window, press c.")
if solver or args.wifi:
    print("Press m to send a stable board to the Pico"
          + (f" over Wi-Fi ({args.wifi})." if args.wifi else "."))
while True:
    ok, frame = cap.read()
    if not ok:
        print("Lost the camera picture.")
        break
    view = frame.copy()
    bv.draw_grid(view)

    if empty_cells is None:
        cv2.putText(view, "EMPTY board in grid, then press c", (10, 25),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.65, (0, 255, 255), 2)
    else:
        board, infos = bv.read_board(frame, empty_cells)
        for (x, y, w, h), mark, inf in zip(bv.cell_boxes(frame), board, infos):
            cx, cy = x + w // 2, y + h // 2
            cv2.putText(view, mark, (cx - 14, cy + 14),
                        cv2.FONT_HERSHEY_SIMPLEX, 1.4, COL[mark], 4)
            if debug:
                txt = f"f{inf['fill']:.2f} c{inf['circ']:.2f} s{inf['solid']:.2f}"
                cv2.putText(view, txt, (x, y + h - 4),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.35, (255, 255, 255), 1)
                cv2.rectangle(view, (x, y), (x + w, y + h), (255, 255, 255), 1)

        # Print only when the board changed and has held still
        same_count = same_count + 1 if board == last_board else 0
        last_board = board
        if same_count == STABLE_FRAMES and board != printed:
            print("\nBoard:\n" + bv.board_text(board))
            printed = board

        nx, no = board.count("X"), board.count("O")
        cv2.putText(view, f"X:{nx}  O:{no}", (10, 25),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)

    cv2.putText(view, "c=calibrate  d=debug  s=snapshot  q=quit",
                (10, view.shape[0] - 12), cv2.FONT_HERSHEY_SIMPLEX, 0.5,
                (255, 255, 255), 1)
    cv2.imshow("Board reader - Team 08", view)

    k = cv2.waitKey(1) & 0xFF
    if k == ord('c'):
        empty_cells = bv.calibrate(frame)
        last_board, same_count, printed = None, 0, None
        print("Calibrated on the empty board.")
    elif k == ord('d'):
        debug = not debug
    elif k == ord('s'):
        cv2.imwrite("board_snapshot.png", frame)
        cv2.imwrite("board_snapshot_marked.png", view)
        print("Saved board_snapshot.png and board_snapshot_marked.png")
    elif k == ord('m') and (solver or args.wifi):
        if last_board is None or same_count < STABLE_FRAMES or "?" in last_board:
            print("Wait for a stable board with all cells recognized.")
            continue
        payload = "".join(last_board)
        if args.wifi:
            try:
                response = wifi.ask(payload, args.wifi)
            except OSError as e:
                response = f"Wi-Fi error: {e}"
        else:
            solver.reset_input_buffer()
            solver.write((payload + "\n").encode("ascii"))
            response = solver.readline().decode("ascii", errors="replace").strip()
        print(f"Pico for {payload}: {response or 'no response'}")
    elif k == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
if solver:
    solver.close()
