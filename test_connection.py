#!/usr/bin/env python3
"""Quick connection test — connects, waits 5s, checks if still alive."""
import socket, time, sys, json

HOST = "120.55.63.32"
PORT = 8080

def test_connection():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(5)
    try:
        sock.connect((HOST, PORT))
        print(f"[OK] Connected to {HOST}:{PORT}")
        time.sleep(2)
        # Try sending a LOGIN message
        msg = json.dumps({"type": "REGISTER", "username": "testuser", "password": "test123"})
        prefix = len(msg).to_bytes(4, 'big')
        sock.sendall(prefix + msg.encode())
        print(f"[OK] Sent REGISTER message ({len(msg)} bytes)")
        time.sleep(1)
        # Try receiving response
        data = sock.recv(4096)
        if data:
            print(f"[OK] Received {len(data)} bytes: {data[:100]}")
        else:
            print("[FAIL] Connection closed by server (recv returned 0)")
            return False
        print("[OK] Connection stable!")
        return True
    except Exception as e:
        print(f"[FAIL] {e}")
        return False
    finally:
        sock.close()

if __name__ == "__main__":
    success = test_connection()
    sys.exit(0 if success else 1)
