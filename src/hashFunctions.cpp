#include <immintrin.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "hashFunctions.h"

uint32_t crc32Wrapper(string *data, uint32_t seed) {
    return crc32(data);
}

uint32_t crc32(string *data) {
    #ifndef OPTIMIZE_CRC32
        char  *message = data->data;
        size_t length  = data->size;

        uint32_t crc = 0xFFFFFFFF;

        for (size_t i = 0; i < length; i++) {
            crc ^= (uint32_t)message[i];

            for (size_t j = 0; j < 8; j++) {
                crc = (crc >> 1) ^ (0x82F63B78 & -(crc & 1)); // crc32 - castagnoli
            }
        }

        return crc ^ 0xFFFFFFFF;
    #else
        uint32_t crc32 = 0xFFFFFFFF;

        unsigned char *buffer = (unsigned char *)data->data;
        size_t         length = data->size;

        size_t quotient = length / 8;

        while (quotient--) {
            crc32 = _mm_crc32_u64(crc32, *(uint64_t *)buffer);
            buffer += 8;
        }

        if (length & 4) {
            crc32 = _mm_crc32_u32(crc32, *(uint32_t *)buffer);
            buffer += 4;
        }

        if (length & 2) {
            crc32 = _mm_crc32_u16(crc32, *(uint16_t *)buffer);
            buffer += 2;
        }

        if (length & 1) {
            crc32 = _mm_crc32_u8(crc32, *(uint8_t *)buffer);
        }

        return crc32 ^ 0xFFFFFFFF;
    #endif
}

uint32_t loadWord(const char *p) {
    return (uint32_t)((uint8_t)p[0])
           | ((uint8_t)p[1] << 8)
           | ((uint8_t)p[2] << 16)
           | ((uint8_t)p[3] << 24);
}

uint32_t rotl32(uint32_t x, int8_t r) {
    return (x << r) | (x >> (32 - r));
}

uint32_t murmur3Wrapper(string *data, uint32_t seed) {
    return murmur3(data, seed);
}

uint32_t murmur3(string *data, uint32_t seed) {
    const char *message = data->data;
    size_t length = data->size;

    const uint32_t c1 = 0xCC9E2D51;
    const uint32_t c2 = 0x1B873593;
    const uint32_t c3 = 0x85EBCA6B;
    const uint32_t c4 = 0xC2B2AE35;

    uint32_t h       = seed;
    size_t   nblocks = length / 4;

    for (size_t i = 0; i < nblocks; i++) {
        uint32_t k = loadWord(message + i * 4);
        k *= c1;
        k = rotl32(k, 15);
        k *= c2;

        h ^= k;
        h = rotl32(h, 13);
        h = h * 5 + 0xE6546B64;
    }

    const uint8_t *tail = (const uint8_t*)(message + nblocks * 4);
    uint32_t k1 = 0;

    switch (length & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
            k1 *= c1;
            k1 = rotl32(k1, 15);
            k1 *= c2;
            h ^= k1;
    }

    h ^= length;
    h ^= h >> 16;
    h *= c3;
    h ^= h >> 13;
    h *= c4;
    h ^= h >> 16;

    return h;
}

uint32_t pjw32Wrapper(string *data, uint32_t seed) {
    return pjw32(data, MOD);
}

uint32_t pjw32(string *data, uint32_t modulo) {
    char  *message = data->data;
    size_t length  = data->size;

    uint32_t hash = 0;
    uint32_t test = 0;

    for (size_t i = 0; i < length; i++) {
        hash = (hash << 4) + (uint8_t)(message[i]);

        if ((test == (hash & 0xF0000000)) != 0) {
            hash = ((hash ^ (test >> 24)) & (0xFFFFFFF));
        }
    }

    return hash % modulo;
}

uint32_t adler32Wrapper(string *data, uint32_t seed) {
    return adler32(data);
}

uint32_t adler32(string *data) {
    const char* message = data->data;
    size_t length = data->size;

    uint32_t a = 1;
    uint32_t b = 0;

    #ifdef OPTIMIZE_ADLER32

    size_t i = 0;

    for (; i + 4 <= length; i += 4) {
        uint32_t bytes;
        memcpy(&bytes, message + i, 4);

        __asm__ volatile (
                "mov    %1,    %%ecx\n"
                "movzbl %%cl,  %%ecx\n"
                "addl   %%ecx, %0\n"
                "addl   %0,    %2\n"
                "mov    %1,    %%ecx\n"
                "shr    $8,    %%ecx\n"
                "movzbl %%cl,  %%ecx\n"
                "addl   %%ecx, %0\n"
                "addl   %0,    %2\n"
                "mov    %1,    %%ecx\n"
                "shr    $16,   %%ecx\n"
                "movzbl %%cl,  %%ecx\n"
                "addl   %%ecx, %0\n"
                "addl   %0,    %2\n"
                "mov    %1,    %%ecx\n"
                "shr    $24,   %%ecx\n"
                "movzbl %%cl,  %%ecx\n"
                "addl   %%ecx, %0\n"
                "addl   %0,    %2\n"
                : "+r"(a), "+r"(bytes), "+r"(b)
                :
                : "ecx"
        );
    }

    for (; i < length; i++) {
        a = (a + (uint8_t)(message[i])) % 65521;
        b = (b + a) % 65521;
    }

    a %= 65521;
    b %= 65521;

    #else
        for (size_t i = 0; i < length; i++) {
            a = (a + (uint8_t)(message[i])) % 65521;
            b = (b + a) % 65521;
        }
    #endif

    return (b << 16) + a;
}

uint32_t fnv32Wrapper(string *data, uint32_t seed) {
    return fnv32(data);
}

uint32_t fnv32(string *data) {
    char  *message = data->data;
    size_t length  = data->size;

    uint32_t hash = 0x811C9DC5;

    for (size_t i = 0; i < length; i++) {
        hash ^= (uint8_t)(message[i]);
        hash *= 16777619;
    }

    return hash;
}

uint32_t sdbm32Wrapper(string *data, uint32_t seed) {
    return sdbm32(data);
}

uint32_t sdbm32(string *data) {
    char  *message = data->data;
    size_t length  = data->size;

    uint32_t hash = 0;

    for (size_t i = 0; i < length; i++) {
        hash = (uint8_t)(message[i]) + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}