# Plenary Session 9 - 03.11.2025 #

## Reliable Transport Protocol over HiP

A simple Go-Back-N reliable transport protocol (RTP) for file transfer over the HiP link-layer protocol.

### Features

- **Go-Back-N ARQ**: Sliding window protocol for reliable delivery.
- **Sequence Numbers**: For packet ordering and duplicate detection.
- **Retransmission**: On duplicate ACKs.
- **Configurable**: Window size and payload are defined in `include/rtp.h`.

### Building

1.  **Compile the program:**
    ```bash
    make all
    ```
2.  **Clean up build files:**
    ```bash
    make clean
    ```

### Usage

1.  **Start Mininet:**
    ```bash
    sudo -E mn --custom topo_p2p.py --topo mytopo --link tc
    ```
2.  **Open terminals for nodes A and B:**
    ```bash
    mininet> xterm A B
    ```
3.  **Run the file transfer:**

    **On Node B (Receiver):**
    ```bash
    cd bin
    sudo ./rtp -l
    ```

    **On Node A (Sender):**
    First, create a test file if you don't have one:
    ```bash
    dd if=/dev/urandom of=testfile.dat bs=1K count=32
    ```
    Then, send it:
    ```bash
    cd bin
    sudo ./rtp -c testfile.dat
    ```

4.  **Verify the transfer on Node B:**
    The received file will be named `received_file_<HIP_ADDR>_received`.
    ```bash
    # Compare checksums
    md5sum testfile.dat received_file_*_received
    ```
