#include "protocol.h"
#include "isocket.h"
#include "file_handler.h"
#include "integrity.h"
#include <cstdio>
#include <cstring>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <filepath>\n", argv[0]);
        return 1;
    }

    const std::string ip   = argv[1];
    const int         port = std::atoi(argv[2]);
    const std::string path = argv[3];

    FileHandler file;
    if (!file.OpenRead(path)) {
        fprintf(stderr, "Error: cannot open '%s'\n", path.c_str());
        return 1;
    }

    uint64_t fsize  = file.FileSize();
    uint64_t chunks = (fsize + CHUNK_SIZE - 1) / CHUNK_SIZE;

    ISocket sock;
    if (!sock.Connect(ip, port)) {
        fprintf(stderr, "Error: cannot connect to %s:%d\n", ip.c_str(), port);
        return 1;
    }
    printf("Connected to %s:%d\n", ip.c_str(), port);

    // 1. Send metadata packet
    FileMetadata meta{};
    const char* fname = strrchr(path.c_str(), '/');
    if (!fname) fname = strrchr(path.c_str(), '\\');
    fname = fname ? fname + 1 : path.c_str();
    strncpy(meta.filename, fname, sizeof(meta.filename) - 1);
    meta.total_size   = fsize;
    meta.total_chunks = chunks;

    PacketHeader hdr{};
    hdr.magic        = MAGIC_NUMBER;
    hdr.type         = PKT_META;
    hdr.payload_size = sizeof(FileMetadata);
    hdr.sequence_id  = 0;
    IntegrityManager::ComputeHash(&meta, sizeof(meta), hdr.checksum);

    sock.Send(&hdr, HEADER_SIZE);
    sock.Send(&meta, sizeof(meta));
    printf("Sent metadata: %s (%llu bytes, %llu chunks)\n",
           meta.filename, (unsigned long long)fsize, (unsigned long long)chunks);

    // 2. Transfer loop
    char buf[CHUNK_SIZE];
    for (uint64_t seq = 1; seq <= chunks; seq++) {
        size_t n = file.ReadChunk(buf, CHUNK_SIZE);
        if (n == 0) break;

        PacketHeader dh{};
        dh.magic        = MAGIC_NUMBER;
        dh.type         = PKT_DATA;
        dh.payload_size = static_cast<uint32_t>(n);
        dh.sequence_id  = seq;
        IntegrityManager::ComputeHash(buf, n, dh.checksum);

        if (sock.Send(&dh, HEADER_SIZE) < 0 || sock.Send(buf, n) < 0) {
            fprintf(stderr, "Error: send failed at seq %llu\n", (unsigned long long)seq);
            return 1;
        }
        printf("\rSent chunk %llu/%llu", (unsigned long long)seq, (unsigned long long)chunks);
        fflush(stdout);
    }

    // 3. Send EOF
    PacketHeader eof{};
    eof.magic       = MAGIC_NUMBER;
    eof.type        = PKT_EOF;
    eof.payload_size = 0;
    eof.sequence_id = chunks + 1;
    memset(eof.checksum, 0, HASH_SIZE);
    sock.Send(&eof, HEADER_SIZE);

    printf("\nTransfer complete.\n");
    return 0;
}
