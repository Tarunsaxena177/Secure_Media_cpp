# Adaptive Networking & Secure Media Transmission Prototype

This project demonstrates a secure media transmission system using C++17, TCP sockets, and OpenSSL. It simulates a producer sending encrypted media frames to a receiver that verifies integrity and authenticity.

## System Architecture

1.  **Producer (Sender)**:
    *   Generates media frames (e.g., `FRAME_1`).
    *   Computes SHA-256 checksum of the payload.
    *   Encrypts the payload using AES-256-CBC.
    *   Serializes data into a custom binary packet.
    *   Transmits packets over a TCP connection.

2.  **Receiver (Server)**:
    *   Listens for incoming TCP connections.
    *   Deserializes incoming binary packets.
    *   Validates the authentication token.
    *   Decrypts the payload using the shared AES key.
    *   Verifies integrity by comparing the received SHA-256 hash with the calculated hash of the decrypted payload.
    *   Detects out-of-order or duplicate packets using `packet_id`.

## Packet Structure

The custom packet is serialized into a binary format:
*   `sender_id`: String identifying the source.
*   `packet_id`: Integer for sequencing and reliability.
*   `timestamp`: 64-bit integer for timing.
*   `authentication_token`: String for simple access control.
*   `payload_hash`: SHA-256 checksum of the plaintext payload.
*   `encrypted_payload`: The AES-256 encrypted media frame.

## Encryption Method

*   **AES-256-CBC**: Used for payload confidentiality.
*   **SHA-256**: Used for payload integrity verification.
*   **OpenSSL EVP API**: High-level interface for cryptographic operations.

## Compilation Instructions

Ensure you have `g++` and `libssl-dev` installed.

### Compile Common Logic
The common logic is included directly in the compilation of the producer and receiver for simplicity in this prototype.

### Compile Producer
```bash
g++ producer/producer.cpp common/packet.cpp common/crypto.cpp -o producer -lssl -lcrypto -lpthread
```

### Compile Receiver
```bash
g++ receiver/receiver.cpp common/packet.cpp common/crypto.cpp -o receiver -lssl -lcrypto -lpthread
```

## Running the Prototype

1.  **Start the Receiver**:
    ```bash
    ./receiver
    ```

2.  **Start the Producer** (in a separate terminal):
    ```bash
    ./producer
    ```

## Sample Output

**Producer:**
```text
Connecting to receiver at 127.0.0.1:8080...
Producer sending packet 1 (FRAME_1)
Producer sending packet 2 (FRAME_2)
...
```

**Receiver:**
```text
Receiver listening on port 8080...

Receiver received packet 1 from PRODUCER_01
Decrypting payload...
Integrity verified (SHA-256 match)
Frame processed: FRAME_1

Receiver received packet 2 from PRODUCER_01
Decrypting payload...
Integrity verified (SHA-256 match)
Frame processed: FRAME_2
```
