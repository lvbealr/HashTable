#include <stdlib.h>

#include "benchmark.h"
#include "hashTable.h"
#include "hashFunctions.h"

// TODO move to func
#define RUN_BENCHMARKS(...) {                                                                       \
    __VA_ARGS__                                                                                     \
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
        benchmarkHashTable(pjw32Wrapper,    inputFile, testFile, "pjw32.csv",   "pjw32");
        benchmarkHashTable(adler32Wrapper,  inputFile, testFile, "adler32.csv", "adler32");
        benchmarkHashTable(sdbm32Wrapper,   inputFile, testFile, "sdbm32.csv",  "sdbm32");
        benchmarkHashTable(fnv32Wrapper,    inputFile, testFile, "fnv32.csv",   "fnv32");
        benchmarkHashTable(murmur3Wrapper,  inputFile, testFile, "murmur3.csv", "murmur3");
        benchmarkHashTable(crc32Wrapper,    inputFile, testFile, "crc32.csv", "crc32");
    )

    return EXIT_SUCCESS;
}