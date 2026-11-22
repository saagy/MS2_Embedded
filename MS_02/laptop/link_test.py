"""Webcam + laptop <-> Pico serial link test (Milestone 2, Team 08).

Usage:  python link_test.py [camera_index] [COMport]
        python link_test.py            -> camera 1, Pico port found automatically

Click the video window, then press:
  r / g / b / o = set the Pico LED red / green / blue / off
  p             = ping the Pico (it answers PONG)
  t             = 3x3 grid on/off
  q             = quit
Pressing the Pico's push button shows "BTN" on the video and in the terminal.

Close the VS Code Serial Monitor first: only one program can use the port.
"""
import sys
import time
import cv2
import serial
from serial.tools import list_ports

PICO_VID = 0x2E8A  # Raspberry Pi


def find_pico_port():
    for p in list_ports.comports():
        if p.vid == PICO_VID:
            return p.device
    return None


cam_index = int(sys.argv[1]) if len(sys.argv) > 1 else 1
port = sys.argv[2] if len(sys.argv) > 2 else find_pico_port()
if not port:
    sys.exit("Pico not found. Plug it in (normally, not BOOTSEL) and try again.")

ser = serial.Serial(port, 115200, timeout=0)
ser.dtr = True  # the Pico only sends USB serial data when DTR is set
cap = cv2.VideoCapture(cam_index, cv2.CAP_DSHOW)
if not cap.isOpened():
    sys.exit(f"Camera {cam_index} not found. Run camera_check.py.")

print(f"Pico on {port}, camera {cam_index}.")
print("Click the video window: r/g/b/o = LED, p = ping, q = quit")

COLOURS = {"RED": (0, 0, 255), "GREEN": (0, 255, 0),
           "BLUE": (255, 0, 0), "OFF": (80, 80, 80)}
led_colour = (80, 80, 80)
last_msg, last_msg_t = "waiting for Pico...", time.time()
btn_count, btn_flash_until = 0, 0.0
show_grid = True
buf = b""

while True:
    ok, frame = cap.read()
    if not ok:
        print("Lost the camera picture.")
        break

    # Read whatever the Pico sent, line by line
    buf += ser.read(256)
    while b"\n" in buf:
        line, buf = buf.split(b"\n", 1)
        msg = line.decode(errors="ignore").strip()
        if not msg:
            continue
        print("Pico:", msg)
        last_msg, last_msg_t = msg, time.time()
        if msg.startswith("ACK "):
            led_colour = COLOURS.get(msg[4:], led_colour)
        if msg == "BTN":
            btn_count += 1
            btn_flash_until = time.time() + 0.5

    # 3x3 grid in a centered square: line the tic-tac-toe board up with it
    if show_grid:
        h, w = frame.shape[:2]
        side = int(min(w, h) * 0.9)
        x0, y0 = (w - side) // 2, (h - side) // 2
        for i in range(4):
            x = x0 + i * side // 3
            y = y0 + i * side // 3
            cv2.line(frame, (x, y0), (x, y0 + side), (0, 255, 0), 2)
            cv2.line(frame, (x0, y), (x0 + side, y), (0, 255, 0), 2)

    # Overlay: LED colour dot, last Pico message, button counter
    cv2.circle(frame, (30, 30), 15, led_colour, -1)
    cv2.putText(frame, f"Pico: {last_msg}", (55, 37),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)
    cv2.putText(frame, f"Button presses: {btn_count}", (10, 70),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
    if time.time() < btn_flash_until:
        cv2.putText(frame, "BUTTON!", (10, 120),
                    cv2.FONT_HERSHEY_SIMPLEX, 1.2, (0, 0, 255), 3)
    cv2.putText(frame, "r/g/b/o = LED  p = ping  t = grid  q = quit",
                (10, frame.shape[0] - 15), cv2.FONT_HERSHEY_SIMPLEX, 0.5,
                (255, 255, 255), 1)

    cv2.imshow("Webcam + Pico link - Team 08", frame)
    k = cv2.waitKey(1) & 0xFF
    if k in (ord('r'), ord('g'), ord('b'), ord('o'), ord('p')):
        ser.write(bytes([k]))
    elif k == ord('t'):
        show_grid = not show_grid
    elif k == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
ser.close()
