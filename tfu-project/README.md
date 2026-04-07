# TCP File Transfer Utility

Cross-platform TCP file transfer utility supporting files up to 16 GB with SHA-256 integrity verification.

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Clean

```bash
rm -rf build
```

## Run

**Receiver** (start first):
```bash
./build/tfu_receiver <port> [output_dir]
# Example: ./build/tfu_receiver 9000 ./received
```

**Sender**:
```bash
./build/tfu_sender <server_ip> <port> <filepath>
# Example: ./build/tfu_sender 127.0.0.1 9000 ./largefile.bin
```

## Test

```bash
cd build && ctest --output-on-failure
```

| Suite | Covers |
|---|---|
| `test_integrity` | SHA-256 known vectors (empty, "abc"), verify match/mismatch, determinism, input divergence |
| `test_file_handler` | Chunked read/write roundtrip, file size, seek, nonexistent file, multi-chunk, EOF |
| `test_protocol` | PacketHeader packed size (49 bytes), FileMetadata 16 GB support, magic number, packet types, binary layout offsets |

## Protocol

- 1 MB chunk size with per-chunk SHA-256 verification
- Binary wire protocol with magic number validation
- Constant memory usage regardless of file size
