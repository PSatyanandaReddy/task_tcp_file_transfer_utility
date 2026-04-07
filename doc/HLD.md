# Design Document: Cross-Platform TCP File Transfer Utility

## 1. Introduction
[cite_start]This utility is a cross-platform tool designed to securely transfer files over a network[cite: 3, 11]. [cite_start]It specifically addresses the challenge of transferring large files (up to 16 GB) between two networked computers using a TCP-based approach[cite: 6, 8].

## 2. Technical Stack
* [cite_start]**Implementation Language:** C/C++ (chosen for economy of system resource utilization and performance)[cite: 4, 18].
* [cite_start]**Build System:** CMake (ensures cross-platform build compatibility)[cite: 11, 22].
* [cite_start]**Networking:** Standard TCP Sockets (POSIX for Linux/macOS, Winsock2 for Windows)[cite: 6, 11].
* [cite_start]**Integrity:** SHA-256 cryptographic hashing[cite: 9].

## 3. Architectural Design

### 3.1 Wire Protocol (The Packet Header)
[cite_start]To maintain a stable stream and handle errors, each data chunk is preceded by a fixed-size binary header[cite: 10, 15].

| Field | Size (Bytes) | Description |
| :--- | :--- | :--- |
| **Magic Number** | 4 | Validates that the packet belongs to this utility. |
| **Packet Type** | 1 | Identifies Metadata (0x01), Data (0x02), or EOF (0x03). |
| **Payload Size** | 4 | Indicates the length of the data chunk following the header. |
| **Sequence ID** | 8 | [cite_start]Used to track chunks and facilitate retries if a chunk is lost or corrupted[cite: 17]. |
| **Checksum** | 32 | [cite_start]SHA-256 hash of the payload for integrity checks[cite: 9]. |

### 3.2 Core Modules
1.  **File Engine:** Responsible for reading/writing files in fixed-size chunks (e.g., 1 MB). [cite_start]This is critical for supporting 16 GB files without exhausting system RAM[cite: 8, 15, 18].
2.  [cite_start]**Network Wrapper:** Provides a cross-platform abstraction for socket operations, handling the differences between Unix and Windows networking stacks[cite: 11].
3.  [cite_start]**Integrity & Security Engine:** Computes and verifies SHA-256 hashes for every transferred chunk to ensure zero data corruption[cite: 3, 9].

## 4. Design Considerations & Strategy

* [cite_start]**File Chunking:** The file is broken into smaller chunks to allow for granular error handling and to keep the memory footprint low[cite: 15, 18].
* [cite_start]**Compression:** (Planned for future optimization) To improve bandwidth utilization, optional zlib/lz4 compression can be applied to chunks before transmission[cite: 12, 16].
* **Error Handling:** The utility implements timeouts and a "NACK" (Negative Acknowledgment) system. [cite_start]If a checksum fails, the receiver requests a retry of that specific sequence ID[cite: 10, 17].
* [cite_start]**Resource Economy:** By using buffered I/O and avoiding "load-the-whole-file" patterns, the utility remains lightweight regardless of the file size[cite: 18].

## 5. Implementation Workflow
1.  **Handshake:** Sender transmits metadata (filename, size, total chunks).
2.  **Streaming:** The sender iterates through the file, chunk by chunk, calculating hashes and sending packets.
3.  **Validation:** The receiver verifies each chunk in real-time.
4.  [cite_start]**Completion:** Once the final chunk is verified, the receiver assembles the file and sends a final acknowledgment[cite: 9, 23].

## 6. Deliverables Checklist
* [cite_start][ ] Source code (C/C++)[cite: 21].
* [cite_start][ ] README with Build/Clean/Run instructions[cite: 22].
* [cite_start][ ] Unit tests for hashing and chunking modules[cite: 23].
* [cite_start][ ] Final Project Description with C4 diagrams and performance metrics[cite: 24].


## 7. Project struct
/tfu-project
├── CMakeLists.txt        # Build instructions 
├── include/              # Headers (.h)
├── src/                  # Implementation (.cpp) [cite: 21]
│   ├── socket/           # Platform-specific networking 
│   ├── crypto/           # SHA-256 logic 
│   └── io/               # File streaming logic [cite: 8]
├── tests/                # Unit tests [cite: 23]
└── README.md             # Usage and build guide