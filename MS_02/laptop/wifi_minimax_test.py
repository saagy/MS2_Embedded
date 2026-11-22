"""wifi_minimax_test.py - send boards to the Pico's Minimax over Wi-Fi (no camera).

Pico must run test_wifi_minimax (Serial Monitor shows "=== READY: ESP at <ip> ...").
Usage:  python wifi_minimax_test.py [ESP_IP]      (default 172.20.10.2)
Type a board as 9 characters row by row: '.' empty, 'O' player, 'X' robot.
  O........   -> MOVE 4   (robot takes the centre)
  O...X...O   -> MOVE 1
Empty line = quit.
"""
import socket
import sys

ESP_IP = sys.argv[1] if len(sys.argv) > 1 else "172.20.10.2"
PORT = 333


def ask(board, ip=ESP_IP, timeout=6):
    """Send one board, return the Pico's one-line answer."""
    with socket.create_connection((ip, PORT), timeout=timeout) as s:
        s.sendall((board + "\n").encode("ascii"))
        reply = b""
        while not reply.endswith(b"\n"):
            chunk = s.recv(64)
            if not chunk:
                break
            reply += chunk
    return reply.decode(errors="replace").strip()


if __name__ == "__main__":
    print(f"Sending boards to {ESP_IP}:{PORT}. Empty line = quit.")
    while True:
        board = input("board> ").strip()
        if not board:
            break
        try:
            print("Pico:", ask(board) or "(no answer)")
        except OSError as e:
            print("Wi-Fi error:", e)
