#ifndef TFU_PROTOCOL_H
#define TFU_PROTOCOL_H

#include <cstdint>
#include <cstring>

static constexpr uint32_t MAGIC_NUMBER  = 0x54465541; // "TFUA"
static constexpr uint8_t  PKT_META      = 0x01;
static constexpr uint8_t  PKT_DATA      = 0x02;
static constexpr uint8_t  PKT_EOF       = 0x03;
static constexpr size_t   CHUNK_SIZE    = 1024 * 1024; // 1 MB
static constexpr size_t   HASH_SIZE     = 32;          // SHA-256

#pragma pack(push, 1)
struct PacketHeader {
    uint32_t magic;
    uint8_t  type;
    uint32_t payload_size;
    uint64_t sequence_id;
    uint8_t  checksum[HASH_SIZE];
};
#pragma pack(pop)

struct FileMetadata {
    char     filename[256];
    uint64_t total_size;
    uint64_t total_chunks;
};

static constexpr size_t HEADER_SIZE = sizeof(PacketHeader); // 49 bytes

#endif
