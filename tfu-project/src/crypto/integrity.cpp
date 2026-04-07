#include "integrity.h"
#include <cstring>

// Minimal SHA-256 implementation (no external library dependency)
namespace {

static const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

inline uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z)  { return (x & y) ^ (~x & z); }
inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
inline uint32_t sig0(uint32_t x) { return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
inline uint32_t sig1(uint32_t x) { return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
inline uint32_t gam0(uint32_t x) { return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
inline uint32_t gam1(uint32_t x) { return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

struct Sha256Ctx {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t  buf[64];
    uint32_t buflen;
};

void sha256_init(Sha256Ctx& c) {
    c.state[0]=0x6a09e667; c.state[1]=0xbb67ae85; c.state[2]=0x3c6ef372; c.state[3]=0xa54ff53a;
    c.state[4]=0x510e527f; c.state[5]=0x9b05688c; c.state[6]=0x1f83d9ab; c.state[7]=0x5be0cd19;
    c.bitlen = 0; c.buflen = 0;
}

void sha256_transform(Sha256Ctx& c, const uint8_t block[64]) {
    uint32_t w[64], a,b,cc,d,e,f,g,h;
    for (int i=0;i<16;i++)
        w[i] = (uint32_t(block[i*4])<<24)|(uint32_t(block[i*4+1])<<16)|(uint32_t(block[i*4+2])<<8)|block[i*4+3];
    for (int i=16;i<64;i++)
        w[i] = gam1(w[i-2]) + w[i-7] + gam0(w[i-15]) + w[i-16];
    a=c.state[0]; b=c.state[1]; cc=c.state[2]; d=c.state[3];
    e=c.state[4]; f=c.state[5]; g=c.state[6]; h=c.state[7];
    for (int i=0;i<64;i++) {
        uint32_t t1 = h + sig1(e) + ch(e,f,g) + K[i] + w[i];
        uint32_t t2 = sig0(a) + maj(a,b,cc);
        h=g; g=f; f=e; e=d+t1; d=cc; cc=b; b=a; a=t1+t2;
    }
    c.state[0]+=a; c.state[1]+=b; c.state[2]+=cc; c.state[3]+=d;
    c.state[4]+=e; c.state[5]+=f; c.state[6]+=g; c.state[7]+=h;
}

void sha256_update(Sha256Ctx& c, const uint8_t* data, size_t len) {
    for (size_t i=0;i<len;i++) {
        c.buf[c.buflen++] = data[i];
        if (c.buflen == 64) { sha256_transform(c, c.buf); c.bitlen += 512; c.buflen = 0; }
    }
}

void sha256_final(Sha256Ctx& c, uint8_t hash[32]) {
    uint32_t i = c.buflen;
    c.buf[i++] = 0x80;
    if (i > 56) { while (i < 64) c.buf[i++] = 0; sha256_transform(c, c.buf); i = 0; }
    while (i < 56) c.buf[i++] = 0;
    c.bitlen += c.buflen * 8;
    for (int j=7;j>=0;j--) c.buf[56+(7-j)] = static_cast<uint8_t>(c.bitlen >> (j*8));
    sha256_transform(c, c.buf);
    for (int j=0;j<8;j++) {
        hash[j*4]   = (c.state[j]>>24)&0xff; hash[j*4+1] = (c.state[j]>>16)&0xff;
        hash[j*4+2] = (c.state[j]>>8)&0xff;  hash[j*4+3] = c.state[j]&0xff;
    }
}

} // anonymous namespace

void IntegrityManager::ComputeHash(const void* data, size_t len, uint8_t out_hash[32]) {
    Sha256Ctx ctx;
    sha256_init(ctx);
    sha256_update(ctx, static_cast<const uint8_t*>(data), len);
    sha256_final(ctx, out_hash);
}

bool IntegrityManager::VerifyHash(const void* data, size_t len, const uint8_t expected_hash[32]) {
    uint8_t computed[32];
    ComputeHash(data, len, computed);
    return std::memcmp(computed, expected_hash, 32) == 0;
}
