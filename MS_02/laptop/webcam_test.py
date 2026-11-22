"""Live view from the USB webcam with a 3x3 grid for aligning the board.

Usage:  python webcam_test.py [camera_index]
Keys (click the video window first):
  q = quit    s = save a snapshot (board_snapshot.png)
  g = grid on/off
"""
import sys
import time
import cv2

index = int(sys.argv[1]) if len(sys.argv) > 1 else 1
cap = cv2.VideoCapture(index, cv2.CAP_DSHOW)
if not cap.isOpened():
    sys.exit(f"Camera {index} not found. Run camera_check.py to see which index works.")

cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

show_grid = True
frames, t0, fps = 0, time.time(), 0.0
print("Click the video window, then: q = quit, s = snapshot, g = grid on/off")

while True:
    ok, frame = cap.read()
    if not ok:
        print("Lost the camera picture.")
        break

    h, w = frame.shape[:2]
    if show_grid:
        # 3x3 grid in a centered square: line the tic-tac-toe board up with it
        side = int(min(w, h) * 0.9)
        x0, y0 = (w - side) // 2, (h - side) // 2
        for i in range(4):
            x = x0 + i * side // 3
            y = y0 + i * side // 3
            cv2.line(frame, (x, y0), (x, y0 + side), (0, 255, 0), 2)
            cv2.line(frame, (x0, y), (x0 + side, y), (0, 255, 0), 2)

    frames += 1
    if time.time() - t0 >= 1.0:
        fps, frames, t0 = frames / (time.time() - t0), 0, time.time()
    cv2.putText(frame, f"cam {index}  {w}x{h}  {fps:.0f} fps", (10, 25),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)

    cv2.imshow("Webcam test - Team 08", frame)
    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    if key == ord('g'):
        show_grid = not show_grid
    if key == ord('s'):
        cv2.imwrite("board_snapshot.png", frame)
        print("Saved board_snapshot.png")

cap.release()
cv2.destroyAllWindows()
