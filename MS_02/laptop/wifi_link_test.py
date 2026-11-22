"""wifi_link_test.py - send a message from the laptop to the ESP-01S over Wi-Fi.

Setup (done once in the Pico's Serial Monitor with test_esp running):
    AT+CIPMUX=1           allow connections
    AT+CIPSERVER=1,333    ESP listens on TCP port 333
Then run on the laptop (laptop on the same hotspot as the ESP):
    python wifi_link_test.py
In the Serial Monitor you should see:  +IPD,0,<len>:hello from laptop
To answer back from the ESP, type in the Serial Monitor:
    AT+CIPSEND=0,5        then, after the '>' prompt:  hi!!!
"""
import socket
import sys

ESP_IP = sys.argv[1] if len(sys.argv) > 1 else "172.20.10.2"
PORT = 333
MESSAGE = "hello from laptop\n"

print(f"Connecting to ESP-01S at {ESP_IP}:{PORT} ...")
try:
    with socket.create_connection((ESP_IP, PORT), timeout=5) as s:
        print("Connected.")
        s.sendall(MESSAGE.encode())
        print(f"Sent: {MESSAGE.strip()!r}")
        print("Waiting up to 30 s for a reply (type AT+CIPSEND=0,5 in the Serial Monitor)...")
        s.settimeout(30)
        try:
            data = s.recv(1024)
            print("Reply from ESP:", data.decode(errors="replace").strip() or "(connection closed)")
        except socket.timeout:
            print("No reply within 30 s (that's fine - sending already worked if the Serial Monitor shows +IPD).")
except OSError as e:
    print("Could not connect:", e)
    print("Check: laptop on the same hotspot, AT+CIPSERVER=1,333 was sent, ESP IP is still", ESP_IP)
