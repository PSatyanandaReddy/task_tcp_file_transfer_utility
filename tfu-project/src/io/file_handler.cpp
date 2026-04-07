#include "file_handler.h"

FileHandler::FileHandler() : fp_(nullptr) {}
FileHandler::~FileHandler() { Close(); }

bool FileHandler::OpenRead(const std::string& path) {
    fp_ = fopen(path.c_str(), "rb");
    return fp_ != nullptr;
}

bool FileHandler::OpenWrite(const std::string& path) {
    fp_ = fopen(path.c_str(), "wb");
    return fp_ != nullptr;
}

size_t FileHandler::ReadChunk(char* buffer, size_t size) {
    if (!fp_) return 0;
    return fread(buffer, 1, size, fp_);
}

bool FileHandler::WriteChunk(const char* buffer, size_t size) {
    if (!fp_) return false;
    return fwrite(buffer, 1, size, fp_) == size;
}

void FileHandler::Seek(uint64_t offset) {
    if (!fp_) return;
#ifdef _WIN32
    _fseeki64(fp_, static_cast<int64_t>(offset), SEEK_SET);
#else
    fseeko(fp_, static_cast<off_t>(offset), SEEK_SET);
#endif
}

uint64_t FileHandler::FileSize() const {
    if (!fp_) return 0;
    auto cur = ftello(fp_);
    fseeko(fp_, 0, SEEK_END);
    auto sz = static_cast<uint64_t>(ftello(fp_));
    fseeko(fp_, cur, SEEK_SET);
    return sz;
}

void FileHandler::Close() {
    if (fp_) { fclose(fp_); fp_ = nullptr; }
}
