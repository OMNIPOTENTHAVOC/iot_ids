#!/bin/bash

echo "--- 1. Generating Robust CA ---"
# Create CA with 'Basic Constraints: CA:TRUE' (Critical for Python)
openssl req -x509 -new -nodes -newkey rsa:4096 -keyout ca.key -sha256 -days 365 -out ca.crt -subj "/CN=IoT_Root_CA" \
  -addext "basicConstraints=critical,CA:TRUE" \
  -addext "keyUsage=critical,keyCertSign,cRLSign"

echo "--- 2. Generating Server Certs ---"
# Server needs 'serverAuth' and SAN (Subject Alternative Name)
openssl req -new -nodes -newkey rsa:2048 -keyout server.key -out server.csr -subj "/CN=localhost"

openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365 -sha256 \
  -extfile <(printf "subjectAltName=DNS:localhost,IP:127.0.0.1\nbasicConstraints=CA:FALSE\nkeyUsage=digitalSignature,keyEncipherment\nextendedKeyUsage=serverAuth")

echo "--- 3. Generating Client Certs ---"
# Client needs 'clientAuth'
openssl req -new -nodes -newkey rsa:2048 -keyout client.key -out client.csr -subj "/CN=python_client_ids"

openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 365 -sha256 \
  -extfile <(printf "basicConstraints=CA:FALSE\nkeyUsage=digitalSignature,keyEncipherment\nextendedKeyUsage=clientAuth")

echo "--- Done! Keys are ready. ---"
