#ifndef HASH_FUNCTIONS_H_
#define HASH_FUNCTIONS_H_

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"

const size_t SEED = 0;
const size_t MOD  = 2048;

typedef uint32_t (*hashFunctionWrapper)(string *, uint32_t);

uint32_t crc32Wrapper   (string *data, uint32_t seed);
uint32_t crc32SSEWrapper(string *data, uint32_t seed);
uint32_t murmur3Wrapper (string *data, uint32_t seed);
uint32_t pjw32Wrapper   (string *data, uint32_t seed);
uint32_t adler32Wrapper (string *data, uint32_t seed);
uint32_t fnv32Wrapper   (string *data, uint32_t seed);
uint32_t sdbm32Wrapper  (string *data, uint32_t seed);

uint32_t crc32   (string *data);
uint32_t crc32SSE(string *data);
uint32_t murmur3 (string *data, uint32_t seed);
uint32_t pjw32   (string *data, uint32_t modulo);
uint32_t adler32 (string *data);
uint32_t fnv32   (string *data);
uint32_t sdbm32  (string *data);

uint32_t loadWord(const char *p);
uint32_t rotl32  (uint32_t x, int8_t r);

#endif // HASH_FUNCTIONS_H_