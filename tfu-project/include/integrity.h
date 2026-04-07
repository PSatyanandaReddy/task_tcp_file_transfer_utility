#ifndef TFU_INTEGRITY_H
#define TFU_INTEGRITY_H

#include <cstdint>
#include <cstddef>

class IntegrityManager {
public:
    static void ComputeHash(const void* data, size_t len, uint8_t out_hash[32]);
    static bool VerifyHash(const void* data, size_t len, const uint8_t expected_hash[32]);
};

#endif
