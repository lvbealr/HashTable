#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "benchmark.h"
#include "IO.h"
#include "hashTable.h"
#include "hashFunctions.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        customPrint(red, bold, bgDefault, "Usage: %s <input-file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *inputFile  = argv[1];
    const char *testFile   = argv[2];

    benchmarkHashTable(pjw32Wrapper,   inputFile, testFile, "pjw32.csv");
    benchmarkHashTable(adler32Wrapper, inputFile, testFile, "adler32.csv");
    benchmarkHashTable(murmur3Wrapper, inputFile, testFile, "murmur3.csv");
    benchmarkHashTable(crc32Wrapper,   inputFile, testFile, "crc32.csv");
    benchmarkHashTable(fnv32Wrapper,   inputFile, testFile, "fnv32.csv");
    benchmarkHashTable(sdbm32Wrapper,  inputFile, testFile, "sdbm32.csv");

    return EXIT_SUCCESS;
}