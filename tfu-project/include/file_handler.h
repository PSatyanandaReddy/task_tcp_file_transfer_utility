#ifndef TFU_FILE_HANDLER_H
#define TFU_FILE_HANDLER_H

#include <cstdint>
#include <cstdio>
#include <string>

class FileHandler {
public:
    FileHandler();
    ~FileHandler();

    bool OpenRead(const std::string& path);
    bool OpenWrite(const std::string& path);
    size_t ReadChunk(char* buffer, size_t size);
    bool   WriteChunk(const char* buffer, size_t size);
    void   Seek(uint64_t offset);
    uint64_t FileSize() const;
    void Close();

private:
    FILE* fp_;
};

#endif
