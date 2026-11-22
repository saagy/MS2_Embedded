"""Find every camera the laptop can open.

Usage:  python camera_check.py
Prints each working camera index and its resolution. The USB webcam is
usually index 1 when the laptop also has a built-in camera.
"""
import cv2

found = []
for index in range(5):
    cap = cv2.VideoCapture(index, cv2.CAP_DSHOW)  # DirectShow: fast on Windows
    if cap.isOpened():
        ok, frame = cap.read()
        if ok:
            h, w = frame.shape[:2]
            print(f"Camera {index}: OK  {w}x{h}")
            found.append(index)
        else:
            print(f"Camera {index}: opens but gives no picture")
    cap.release()

if not found:
    print("No camera found. Check the USB cable and close other apps using the camera.")
else:
    print(f"\nUse:  python webcam_test.py {found[-1]}   (last one is usually the USB webcam)")
