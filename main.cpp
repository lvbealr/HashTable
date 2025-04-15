#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "IO.h"
#include "hashTable.h"
#include "hashFunctions.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        customPrint(red, bold, bgDefault, "Usage: %s <input-file> <output-csv-file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *inputFile  = argv[1];
    const char *outputFile = argv[2];

    textData textData = {};
    textDataInitialize(inputFile, &textData);

    hashTable<string *> hashTable = {};
    initializeHashTable(&hashTable, 2);

    fillHashTableWithRehash(&hashTable, &textData, crc32);

    int openFile = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    customWarning(openFile != -1, EXIT_FAILURE);

    const size_t MAX_LINE_SIZE = 256;
    char *buffer = (char *)calloc(hashTable.capacity, MAX_LINE_SIZE);
    char *currPtr = buffer;

    for (size_t i = 0; i < hashTable.capacity; i++) {
        int written = snprintf(currPtr, MAX_LINE_SIZE, "%lu,%zd\n", i, hashTable.table[i].size);

        if (written < 0) {
            customPrint(red, bold, bgDefault, "Failed to format string\n");
            break;
        }
        currPtr += written;
    }

    ssize_t writeFile = write(openFile, buffer, currPtr - buffer);
    customWarning(writeFile != -1, EXIT_FAILURE);

    close(openFile);
    free(buffer);

    textDataDestruct(&textData);
    destroyHashTable(&hashTable);

    return EXIT_SUCCESS;
}