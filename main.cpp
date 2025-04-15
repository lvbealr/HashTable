#include <stdlib.h>
#include <stdio.h>
#include "IO.h"
#include "hashTable.h"
#include "hashFunctions.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        customPrint(red, bold, bgDefault, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *file = argv[--argc];

    textData textData = {};
    textDataInitialize(file, &textData);

    hashTable<string *> hashTable = {};
    initializeHashTable(&hashTable, 8);

    fillHashTableWithRehash(&hashTable, &textData, crc32);

    for (size_t i = 0; i < hashTable.capacity; i++) {
        printf("[%lu] -> size [%zd]\n", i, hashTable.table[i].size);
    }

    textDataDestruct(&textData);

    destroyHashTable(&hashTable);

    return EXIT_SUCCESS;
}