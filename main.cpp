#include <stdlib.h>

#include "benchmark.h"
#include "hashTable.h"
#include "hashFunctions.h"
#include "include/benchmark.h"

#define RUN_BENCHMARKS(...) { \
    __VA_ARGS__               \
}

#define RUN_TEST(ITER, ...) {               \
    for (size_t i = 0; i < ITER; i++) {     \
        __VA_ARGS__                         \
    }                                       \
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        customPrint(red, bold, bgDefault, "Usage: %s <input-file> <test-file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *inputFile  = argv[1];
    const char *testFile   = argv[2];

    // TODO (optional) pass hash func as a template parameter

    RUN_BENCHMARKS(
        benchmarkHashTable(pjw32Wrapper,    inputFile, testFile, 
            "pjw32.csv",   "pjw32",   benchmarkMode::BENCHMARK_MODE_WARMING_UP);
        RUN_TEST(100, benchmarkHashTable(pjw32Wrapper,    inputFile, testFile, 
            "pjw32.csv",   "pjw32",   benchmarkMode::BENCHMARK_MODE_TIME);)
        
        benchmarkHashTable(adler32Wrapper,  inputFile, testFile, 
            "adler32.csv", "adler32", benchmarkMode::BENCHMARK_MODE_WARMING_UP);
        RUN_TEST(100, benchmarkHashTable(adler32Wrapper,  inputFile, testFile, 
            "adler32.csv", "adler32", benchmarkMode::BENCHMARK_MODE_TIME);)

        benchmarkHashTable(sdbm32Wrapper,   inputFile, testFile, 
            "sdbm32.csv",  "sdbm32",  benchmarkMode::BENCHMARK_MODE_WARMING_UP);    
        RUN_TEST(100, benchmarkHashTable(sdbm32Wrapper,   inputFile, testFile, 
            "sdbm32.csv",  "sdbm32",  benchmarkMode::BENCHMARK_MODE_TIME);)
        
        benchmarkHashTable(fnv32Wrapper,    inputFile, testFile, 
            "fnv32.csv",   "fnv32",   benchmarkMode::BENCHMARK_MODE_WARMING_UP);
        RUN_TEST(100, benchmarkHashTable(fnv32Wrapper,    inputFile, testFile, 
            "fnv32.csv",   "fnv32",   benchmarkMode::BENCHMARK_MODE_TIME);)
        
        benchmarkHashTable(murmur3Wrapper,  inputFile, testFile, 
            "murmur3.csv", "murmur3", benchmarkMode::BENCHMARK_MODE_WARMING_UP);
        RUN_TEST(100, benchmarkHashTable(murmur3Wrapper,  inputFile, testFile, 
            "murmur3.csv", "murmur3", benchmarkMode::BENCHMARK_MODE_TIME);)

        benchmarkHashTable(crc32Wrapper,    inputFile, testFile, 
            "crc32.csv", "crc32",     benchmarkMode::BENCHMARK_MODE_WARMING_UP);    
        RUN_TEST(100, benchmarkHashTable(crc32Wrapper,    inputFile, testFile, 
            "crc32.csv", "crc32",     benchmarkMode::BENCHMARK_MODE_TIME);)
    )

    return EXIT_SUCCESS;
}