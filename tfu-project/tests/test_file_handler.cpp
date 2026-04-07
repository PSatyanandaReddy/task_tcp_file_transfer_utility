#include "test.h"
#include "file_handler.h"
#include <cstring>
#include <cstdio>

static const char* TMP_FILE = "/tmp/tfu_test_fh.bin";

void test_write_and_read() {
    FileHandler w;
    ASSERT_TRUE(w.OpenWrite(TMP_FILE));
    const char data[] = "hello chunked world";
    ASSERT_TRUE(w.WriteChunk(data, sizeof(data)));
    w.Close();

    FileHandler r;
    ASSERT_TRUE(r.OpenRead(TMP_FILE));
    char buf[64] = {};
    size_t n = r.ReadChunk(buf, sizeof(buf));
    ASSERT_EQ(n, sizeof(data));
    ASSERT_EQ(memcmp(buf, data, sizeof(data)), 0);
    r.Close();
}

void test_file_size() {
    FileHandler w;
    w.OpenWrite(TMP_FILE);
    char block[1024];
    memset(block, 'A', sizeof(block));
    w.WriteChunk(block, sizeof(block));
    w.Close();

    FileHandler r;
    r.OpenRead(TMP_FILE);
    ASSERT_EQ(r.FileSize(), 1024u);
    r.Close();
}

void test_seek_and_read() {
    FileHandler w;
    w.OpenWrite(TMP_FILE);
    w.WriteChunk("AAAABBBB", 8);
    w.Close();

    FileHandler r;
    r.OpenRead(TMP_FILE);
    r.Seek(4);
    char buf[4] = {};
    size_t n = r.ReadChunk(buf, 4);
    ASSERT_EQ(n, 4u);
    ASSERT_EQ(memcmp(buf, "BBBB", 4), 0);
    r.Close();
}

void test_open_nonexistent() {
    FileHandler r;
    ASSERT_FALSE(r.OpenRead("/tmp/tfu_no_such_file_xyz.bin"));
}

void test_multi_chunk_write() {
    FileHandler w;
    w.OpenWrite(TMP_FILE);
    w.WriteChunk("AAAA", 4);
    w.WriteChunk("BBBB", 4);
    w.WriteChunk("CCCC", 4);
    w.Close();

    FileHandler r;
    r.OpenRead(TMP_FILE);
    ASSERT_EQ(r.FileSize(), 12u);
    char buf[12] = {};
    r.ReadChunk(buf, 12);
    ASSERT_EQ(memcmp(buf, "AAAABBBBCCCC", 12), 0);
    r.Close();
}

void test_read_returns_zero_at_eof() {
    FileHandler w;
    w.OpenWrite(TMP_FILE);
    w.WriteChunk("XY", 2);
    w.Close();

    FileHandler r;
    r.OpenRead(TMP_FILE);
    char buf[64];
    r.ReadChunk(buf, 64); // reads 2 bytes, hits EOF
    size_t n = r.ReadChunk(buf, 64); // should return 0
    ASSERT_EQ(n, 0u);
    r.Close();
}

int main() {
    printf("test_file_handler\n");
    RUN_TEST(test_write_and_read);
    RUN_TEST(test_file_size);
    RUN_TEST(test_seek_and_read);
    RUN_TEST(test_open_nonexistent);
    RUN_TEST(test_multi_chunk_write);
    RUN_TEST(test_read_returns_zero_at_eof);
    remove(TMP_FILE);
    TEST_REPORT();
}
