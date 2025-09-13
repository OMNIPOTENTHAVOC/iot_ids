# Secure IoT IDS + Blockchain Logging System
**Project:** ML-based Intrusion Detection with Blockchain-backed Log Integrity  
**Communication:** Python IDS ↔ C++ Log Manager via mTLS over TCP  
**Author:** SIDDHANSH SRIVASTAVA 

---

## Quick Flowchart

            ┌────────────┐
            │ Python IDS │
            └─────┬──────┘
                  │
   (Feature Extraction & ML)
                  │
                  ▼
            ┌───────────────┐
            │ C++ Log Server│
            └─────┬─────────┘
                  │
      (Verify mTLS, parse)
                  │
                  ▼
            ┌───────────────┐
            │ Log Store     │
            │ (append-only) │
            └─────┬─────────┘
                  │
                  ▼
            ┌───────────────┐
            │ Merkle Builder│
            │ / Signer      │
            └─────┬─────────┘
                  │
                  ▼
            ┌───────────────┐
            │ Anchor to     │
            │ Blockchain    │
            └─────┬─────────┘
                  │
                  ▼
            ┌───────────────┐
            │ Verifier /    │
            │ Forensics     │
            └───────────────┘
---

## Phase 0 — Prerequisites
**What**
- Hardware: development machine(s), Raspberry Pi (for gateway testing).  
- Software: Python 3.x, C++17 toolchain, OpenSSL, Boost.Asio, ML libraries (scikit-learn, TensorFlow, ONNX), Docker, local blockchain node.  
- Documentation: project design doc, threat model, config templates.

**How**
- Create project repo with directories:  
  - `python_ids/`, `cpp_log_server/`, `certs/`, `blockchain/`, `tests/`, `docs/`.  
- Prepare sample datasets and minimal ML model exports.

**Why**
- Ensures consistent environment and repeatable experiments.

**Acceptance / Artifacts**
- Git repo scaffolded with README and directory structure.  
- Dockerfile placeholders for containerization.

---

## Phase 1 — PKI & mTLS Foundation
**What**
- Establish minimal PKI, issue client/server certificates, define TLS policy.

**How**
- Create CA certificate/key.  
- Issue:  
  - Server certificate/key for C++ Log Server.  
  - Client certificate/key for Python IDS.  
- TLS configuration: TLS 1.3, AEAD ciphers (AES-GCM / ChaCha20-Poly1305).  
- Enforce certificate validation (CA, CN/SAN, not-before/not-after).  
- Store private keys securely.

**Why**
- Mutual authentication and encryption are critical to secure IPC.

**Acceptance / Artifacts**
- CA cert + server/client certs stored in `/certs/`.  
- `tls-policy.md` listing TLS version, ciphers, validity periods, revocation plan.

---

## Phase 2 — IPC Protocol & Message Format
**What**
- Define structured messages and message framing.

**How**
- Message schema (JSON or Protobuf):  
  - `batch_id`, `entry_id`, `timestamp`, `device_id`, `alert_type`, `features_summary`, `confidence`, `seq_no`.  
- Framing: 4-byte length prefix.  
- Replay protection: `seq_no` + `timestamp`.  
- Optional lightweight signature for defense-in-depth.

**Why**
- Structured, framed messages prevent ambiguity, replay, and parsing errors.

**Acceptance / Artifacts**
- `message_schema.md` with examples.  
- `framing_spec.md` describing length-prefix format.

---

## Phase 3 — C++ Log Server (mTLS Server)
**What**
- Accept mTLS connections, verify client, parse messages, write logs, enqueue for Merkle tree.

**How**
- TLS server socket requiring client cert verification.  
- Parse framed messages, validate certificate, `seq_no`, timestamp, JSON schema.  
- Append valid messages to append-only log store.  
- Enqueue messages for Merkle tree construction.  

**Why**
- Server is authoritative logger: fast, robust, secure.

**Acceptance / Artifacts**
- C++ server binary that:  
  - accepts mTLS connections,  
  - rejects invalid certs,  
  - appends logs and queues them for Merkle batching,  
  - exposes logs directory and health endpoint (local only).

---

## Phase 4 — Python IDS (mTLS Client)
**What**
- Capture IoT traffic, run ML model, send alerts securely to C++ server.

**How**
- Feature extraction via `scapy` or `pyshark`.  
- ML inference using exported model.  
- Generate structured message, increment `seq_no`.  
- Open mTLS client socket with client cert, send framed message.  
- Maintain local buffer if server unavailable.

**Why**
- Python is ideal for ML and rapid iteration; mTLS ensures transport security.

**Acceptance / Artifacts**
- Python client script:  
  - completes TLS handshake,  
  - sends valid messages,  
  - handles offline buffering.

---

## Phase 5 — Merkle Builder & Anchoring
**What**
- Batch alerts into Merkle trees, sign roots, anchor on blockchain.

**How**
- Compute SHA-256 leaf hashes, construct Merkle tree, produce root.  
- Sign root with server/gateway key.  
- Prepare anchor record: `{batch_id, merkle_root, timestamp, signer_pubkey, signature}`.  
- Submit anchor to blockchain and store tx hash + block number.  
- Ensure idempotency.

**Why**
- Minimizes on-chain storage while enabling per-entry verification.

**Acceptance / Artifacts**
- Merkle builder library, signed anchor records, anchored metadata.

---

## Phase 6 — Blockchain Anchoring Module
**What**
- Submit anchor records reliably to blockchain.

**How**
- Controlled blockchain account with private key.  
- Submit Merkle root in transaction; wait for confirmation.  
- Retry/backoff for network issues; persist pending anchors.

**Why**
- Anchor persistence ensures tamper-evident log verification.

**Acceptance / Artifacts**
- On-chain event logged for a sample root; metadata persisted.

---

## Phase 7 — Verifier & Forensics
**What**
- Verify log entries using Merkle proofs and on-chain anchors.

**How**
- Fetch anchor record.  
- Recompute Merkle root from stored logs.  
- Validate entry via Merkle proof and signer signature.  
- Output PASS/FAIL with explanation.

**Why**
- Enables independent verification and forensic analysis.

**Acceptance / Artifacts**
- Verifier utility outputs verification results correctly.

---

## Phase 8 — Security Hardening & Operational Controls
**What**
- Lock down keys, TLS config, replay protections, logging, monitoring.

**How**
- Enforce TLS 1.3, strong ciphers, strict certificate validation.  
- Secure key storage (permissions, TPM/HSM optional).  
- CRL/OCSP revocation strategy.  
- Rate limiting and audit logs.  
- Time synchronization via NTP.

**Why**
- Protects against replay, spoofing, DoS, and ensures proof validity.

**Acceptance / Artifacts**
- `security-hardening.md` detailing config and TLS validation.

---

## Phase 9 — Testing Plan
**What**
- Unit, integration, security, and performance tests.

**How**
- Unit tests for Merkle, hash, framing parser.  
- Integration tests: Python IDS + C++ server with test certs.  
- Security tests: invalid certs, MITM, replay attacks, tampered logs.  
- Performance tests: latency, throughput, CPU/memory.  
- Fault tolerance: blockchain outage simulations.

**Why**
- Ensures correctness, security, and reliability before deployment.

**Acceptance / Artifacts**
- Test reports, latency numbers, security test pass/fail results.

---

## Phase 10 — Deployment & Operations
**What**
- Containerization, monitoring, backups, key rotation, runbooks.

**How**
- Docker-compose for dev; systemd/K8s for production.  
- Monitor handshake failures, messages/sec, queue length, anchor status.  
- Backup keys, anchors, logs securely.  
- Key rotation procedures tested.  
- Incident response runbook for compromised gateway.

**Why**
- Ensures maintainability, observability, and resilience.

**Acceptance / Artifacts**
- Deployment manifest, monitoring dashboard screenshots, incident runbook.

---

## Suggested Defaults & Rationale
- TLS 1.3, AEAD ciphers.  
- Batch size: 50 or interval: 60s.  
- Replay window: ±30s, monotonic `seq_no`.  
- Log format: NDJSON.  
- Merkle hash: SHA-256 canonical over sorted JSON keys.  
- Anchor target: private blockchain for development; evaluate L2/mainnet for cost.

---

## Final Checklist
- [ ] PKI created and stored in `/certs/`.  
- [ ] C++ mTLS server accepts only client cert signed by CA.  
- [ ] Python mTLS client handshake tested.  
- [ ] Message schema + framing implemented.  
- [ ] Append-only log store verified.  
- [ ] Merkle builder produces reproducible roots.  
- [ ] Anchors recorded on blockchain.  
- [ ] Verifier detects tampering.  
- [ ] Security hardening complete (TLS, keys, replay).  
- [ ] Monitoring & runbooks available.

---

## Elevator Pitch / Summary
**What**: Secure IPC via mTLS between Python IDS and C++ Log Manager, Merkle tree batching, blockchain anchoring.  
**How**: Mutual authentication, encrypted channel, structured messages, append-only logs, batched Merkle roots anchored on-chain.  
**Why**: Prevents spoofing/forging alerts, protects logs in transit, guarantees tamper-evidence, modular design enables independent IDS/logging improvements.

---
