#include "test.h"
#include "protocol.h"
#include <cstring>

void test_header_size() {
    // 4 (magic) + 1 (type) + 4 (payload_size) + 8 (sequence_id) + 32 (checksum) = 49
    ASSERT_EQ(sizeof(PacketHeader), 49u);
    ASSERT_EQ(HEADER_SIZE, 49u);
}

void test_metadata_fields() {
    FileMetadata m{};
    ASSERT_EQ(sizeof(m.filename), 256u);
    m.total_size   = 17179869184ULL; // 16 GB
    m.total_chunks = m.total_size / CHUNK_SIZE;
    ASSERT_EQ(m.total_chunks, 16384u);
}

void test_magic_number() {
    ASSERT_EQ(MAGIC_NUMBER, 0x54465541u);
}

void test_packet_types() {
    ASSERT_EQ(PKT_META, 0x01);
    ASSERT_EQ(PKT_DATA, 0x02);
    ASSERT_EQ(PKT_EOF,  0x03);
}

void test_header_packed_layout() {
    PacketHeader h{};
    h.magic       = MAGIC_NUMBER;
    h.type        = PKT_DATA;
    h.payload_size = 0x1000;
    h.sequence_id = 42;

    auto* raw = reinterpret_cast<uint8_t*>(&h);
    // magic at offset 0
    uint32_t m;
    memcpy(&m, raw, 4);
    ASSERT_EQ(m, MAGIC_NUMBER);
    // type at offset 4
    ASSERT_EQ(raw[4], PKT_DATA);
    // payload_size at offset 5
    uint32_t ps;
    memcpy(&ps, raw + 5, 4);
    ASSERT_EQ(ps, 0x1000u);
    // sequence_id at offset 9
    uint64_t seq;
    memcpy(&seq, raw + 9, 8);
    ASSERT_EQ(seq, 42u);
}

int main() {
    printf("test_protocol\n");
    RUN_TEST(test_header_size);
    RUN_TEST(test_metadata_fields);
    RUN_TEST(test_magic_number);
    RUN_TEST(test_packet_types);
    RUN_TEST(test_header_packed_layout);
    TEST_REPORT();
}
