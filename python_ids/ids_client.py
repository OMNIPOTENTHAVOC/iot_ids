import socket
import ssl
import struct
import json
import time

# Configuration
SERVER_HOST = 'localhost'
SERVER_PORT = 8888
CERT_DIR = '../certs'

def create_secure_client():
    # 1. Create SSL Context for Client
    context = ssl.create_default_context(ssl.Purpose.SERVER_AUTH, cafile=f"{CERT_DIR}/ca.crt")
    
    # 2. Load Client Cert/Key (for mTLS identity)
    context.load_cert_chain(certfile=f"{CERT_DIR}/client.crt", keyfile=f"{CERT_DIR}/client.key")
    
    # 3. Connect
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    secure_sock = context.wrap_socket(sock, server_hostname=SERVER_HOST)
    
    try:
        secure_sock.connect((SERVER_HOST, SERVER_PORT))
        print(f"[Client] Connected to {SERVER_HOST}:{SERVER_PORT} via mTLS!")
        return secure_sock
    except Exception as e:
        print(f"[Client] Connection failed: {e}")
        return None

def send_alert(sock, alert_data):
    # Convert Dict to JSON string
    json_str = json.dumps(alert_data)
    data_bytes = json_str.encode('utf-8')
    
    # Framing: 4-byte length prefix (Big Endian) + Data
    # This matches the C++ ntohl() logic
    packed_len = struct.pack('>I', len(data_bytes))
    
    sock.sendall(packed_len + data_bytes)
    print(f"[Client] Sent alert: {alert_data['alert_type']}")

if __name__ == "__main__":
    client_sock = create_secure_client()
    
    if client_sock:
        seq_no = 1
        try:
            while True:
                # Simulate an IDS Alert (Phase 2 Schema)
                alert = {
                    "batch_id": "batch_001",
                    "timestamp": time.time(),
                    "alert_type": "SYN_FLOOD",
                    "confidence": 0.98,
                    "seq_no": seq_no
                }
                
                send_alert(client_sock, alert)
                seq_no += 1
                time.sleep(2) # Send one every 2 seconds
                
        except KeyboardInterrupt:
            print("[Client] Stopping...")
            client_sock.close()
