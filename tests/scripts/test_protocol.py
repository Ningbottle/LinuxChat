#!/usr/bin/env python3
"""Test with proper 4-byte big-endian length prefix protocol."""
import socket, time, sys, json, struct

HOST = "120.55.63.32"
PORT = 8080

def send_msg(sock, msg_dict):
    """Send a message with 4-byte big-endian length prefix."""
    body = json.dumps(msg_dict).encode('utf-8')
    prefix = struct.pack('!I', len(body))  # 4-byte big-endian
    sock.sendall(prefix + body)
    return len(body)

def recv_msg(sock, timeout=5):
    """Receive a message with 4-byte big-endian length prefix."""
    sock.settimeout(timeout)
    # Read 4-byte length prefix
    data = b''
    while len(data) < 4:
        chunk = sock.recv(4 - len(data))
        if not chunk:
            return None
        data += chunk
    body_len = struct.unpack('!I', data)[0]
    print(f"[DEBUG] Received length prefix: {body_len}")
    # Read body
    body = b''
    while len(body) < body_len:
        chunk = sock.recv(body_len - len(body))
        if not chunk:
            return None
        body += chunk
    return json.loads(body.decode('utf-8'))

def test():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(10)
    try:
        sock.connect((HOST, PORT))
        print(f"[OK] Connected to {HOST}:{PORT}")
        time.sleep(0.5)

        # Send REGISTER message
        msg = {"type": "REGISTER", "username": "testuser", "password": "test123"}
        body_len = send_msg(sock, msg)
        print(f"[OK] Sent REGISTER ({body_len} bytes)")

        # Wait for response
        time.sleep(1)
        try:
            resp = recv_msg(sock, timeout=3)
            if resp:
                print(f"[OK] Received response: {resp}")
            else:
                print("[FAIL] No response or connection closed")
                return False
        except Exception as e:
            print(f"[FAIL] recv error: {e}")
            return False

        print("[OK] Protocol test passed!")
        return True
    except Exception as e:
        print(f"[FAIL] {e}")
        return False
    finally:
        sock.close()

if __name__ == "__main__":
    success = test()
    sys.exit(0 if success else 1)
