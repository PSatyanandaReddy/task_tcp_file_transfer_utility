#include "protocol.h"
#include "isocket.h"
#include "file_handler.h"
#include "integrity.h"
#include <cstdio>
#include <cstring>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <port> [output_dir]\n", argv[0]);
        return 1;
    }

    const int         port = std::atoi(argv[1]);
    const std::string odir = (argc == 3) ? argv[2] : ".";

    ISocket server;
    if (!server.Listen(port)) {
        fprintf(stderr, "Error: cannot listen on port %d\n", port);
        return 1;
    }
    printf("Listening on port %d...\n", port);

    ISocket* client = server.Accept();
    if (!client) {
        fprintf(stderr, "Error: accept failed\n");
        return 1;
    }
    printf("Client connected.\n");

    // 1. Read metadata
    PacketHeader hdr{};
    if (client->Receive(&hdr, HEADER_SIZE) != static_cast<int>(HEADER_SIZE) ||
        hdr.magic != MAGIC_NUMBER || hdr.type != PKT_META) {
        fprintf(stderr, "Error: invalid metadata header (magic=0x%08X type=0x%02X)\n", hdr.magic, hdr.type);
        delete client;
        return 1;
    }

    FileMetadata meta{};
    if (client->Receive(&meta, sizeof(meta)) != static_cast<int>(sizeof(meta))) {
        fprintf(stderr, "Error: failed to read metadata payload\n");
        delete client;
        return 1;
    }

    if (!IntegrityManager::VerifyHash(&meta, sizeof(meta), hdr.checksum)) {
        fprintf(stderr, "Error: metadata checksum mismatch\n");
        delete client;
        return 1;
    }

    printf("Receiving: %s (%llu bytes, %llu chunks)\n",
           meta.filename, (unsigned long long)meta.total_size, (unsigned long long)meta.total_chunks);

    std::string outpath = odir + "/" + meta.filename;
    FileHandler file;
    if (!file.OpenWrite(outpath)) {
        fprintf(stderr, "Error: cannot create '%s'\n", outpath.c_str());
        delete client;
        return 1;
    }

    // 2. Ingestion loop
    char buf[CHUNK_SIZE];
    uint64_t received = 0;

    while (true) {
        PacketHeader ph{};
        int r = client->Receive(&ph, HEADER_SIZE);
        if (r != static_cast<int>(HEADER_SIZE)) {
            fprintf(stderr, "\nError: connection lost (last seq %llu)\n", (unsigned long long)received);
            break;
        }

        if (ph.magic != MAGIC_NUMBER) {
            fprintf(stderr, "\nError: invalid magic number, dropping connection\n");
            break;
        }

        if (ph.type == PKT_EOF) {
            printf("\nEOF received.\n");
            break;
        }

        if (ph.type != PKT_DATA || ph.payload_size > CHUNK_SIZE) {
            fprintf(stderr, "\nError: unexpected packet type 0x%02X\n", ph.type);
            break;
        }

        if (client->Receive(buf, ph.payload_size) != static_cast<int>(ph.payload_size)) {
            fprintf(stderr, "\nError: incomplete chunk at seq %llu\n", (unsigned long long)ph.sequence_id);
            break;
        }

        if (!IntegrityManager::VerifyHash(buf, ph.payload_size, ph.checksum)) {
            fprintf(stderr, "\nError: checksum failure at seq %llu\n", (unsigned long long)ph.sequence_id);
            break;
        }

        uint64_t offset = (ph.sequence_id - 1) * CHUNK_SIZE;
        file.Seek(offset);
        if (!file.WriteChunk(buf, ph.payload_size)) {
            fprintf(stderr, "\nError: disk write failed at seq %llu\n", (unsigned long long)ph.sequence_id);
            break;
        }

        received = ph.sequence_id;
        printf("\rReceived chunk %llu/%llu", (unsigned long long)received, (unsigned long long)meta.total_chunks);
        fflush(stdout);
    }

    file.Close();
    delete client;
    printf("Transfer complete. File saved to: %s\n", outpath.c_str());
    return 0;
}
