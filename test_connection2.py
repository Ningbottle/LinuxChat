#!/usr/bin/env python3
"""Minimal connection test — connect and wait, no data sent."""
import socket, time, sys

HOST = "120.55.63.32"
PORT = 8080

def test():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(10)
    try:
        sock.connect((HOST, PORT))
        print(f"[OK] Connected to {HOST}:{PORT}")
        # Just wait and see if connection stays alive
        for i in range(5):
            time.sleep(1)
            print(f"[OK] {i+1}s elapsed, connection still alive")
        print("[OK] Connection stable for 5 seconds!")
        return True
    except Exception as e:
        print(f"[FAIL] Connection died: {e}")
        return False
    finally:
        sock.close()

if __name__ == "__main__":
    success = test()
    sys.exit(0 if success else 1)
