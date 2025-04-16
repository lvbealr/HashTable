#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#include <fcntl.h>
#include <immintrin.h>
#include <unistd.h>

#include "IO.h"
#include "hashTable.h"
#include "hashFunctions.h"

const size_t MAX_LINE_SIZE = 256;

#define TIME(...) do {                                          \
    uint64_t start = __rdtsc();                                 \
    __VA_ARGS__                                                 \
    uint64_t end   = __rdtsc();                                 \
                                                                \
    uint64_t ticks = end - start;                               \
    customPrint(purple, bold, bgDefault,                        \
    "%-16s: %lu\n", hashFunctionName, ticks);                   \
} while (0)

hashTableError benchmarkHashTable(hashFunctionWrapper hashWrapper,
                                  const char *inputFile, const char *testFile, const char *outputFile,
                                  const char *hashFunctionName);

hashTableError saveHashTableToFile(hashTable<string *> *table, const char *outputFile);
hashTableError searchData         (hashTable<string *> *table, textData *testData, hashFunctionWrapper hashWrapper);

hashTableError benchmarkHashTable(hashFunctionWrapper hashWrapper,
                                  const char *inputFile, const char *testFile, const char *outputFile,
                                  const char *hashFunctionName) {
    customWarning(inputFile,  hashTableError::FILE_NOT_FOUND);
    customWarning(testFile,   hashTableError::FILE_NOT_FOUND);
    customWarning(outputFile, hashTableError::FILE_NOT_FOUND);

    textData inputData = {};
    textDataInitialize(inputFile, &inputData);

    textData testData = {};
    textDataInitialize(testFile, &testData);

    hashTable<string *> hashTable = {};
    initializeHashTable(&hashTable, 2);

    fillHashTableWithRehash(&hashTable, &inputData, hashWrapper);

    customWarning(saveHashTableToFile(&hashTable, outputFile) == hashTableError::NO_ERRORS,
                  hashTableError::SAVE_HASH_TABLE_ERROR);

    TIME(
            customWarning(searchData(&hashTable, &testData, hashWrapper) == hashTableError::NO_ERRORS,
                  hashTableError::SEARCH_DATA_ERROR);
    );

    textDataDestruct(&inputData);
    textDataDestruct(&testData);

    destroyHashTable(&hashTable);

    return hashTableError::NO_ERRORS;
}

hashTableError saveHashTableToFile(hashTable<string *> *table, const char *outputFile) {
    customWarning(table,      hashTableError::HASH_TABLE_BAD_POINTER);
    customWarning(outputFile, hashTableError::FILE_NOT_FOUND);

    int openFile = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    customWarning(openFile != (int)hashTableError::FILE_NOT_FOUND, hashTableError::FILE_NOT_FOUND);

    char *buffer = (char *)calloc(table->capacity, MAX_LINE_SIZE);
    customWarning(buffer, hashTableError::BUFFER_ERROR);

    char *currPtr = buffer;

    for (size_t i = 0; i < table->capacity; i++) {
        int written = snprintf(currPtr, MAX_LINE_SIZE, "%lu,%zd\n", i, table->table[i].size);

        if (written < 0) {
            customPrint(red, bold, bgDefault, "Failed to format string\n");
            break;
        }

        currPtr += written;
    }

    ssize_t writeFile = write(openFile, buffer, (size_t)(currPtr - buffer));
    customWarning(writeFile != (ssize_t)hashTableError::FILE_NOT_FOUND, hashTableError::FILE_NOT_FOUND);

    close(openFile);
    FREE_(buffer);

    return hashTableError::NO_ERRORS;
}

hashTableError searchData(hashTable<string *> *table, textData *testData, hashFunctionWrapper hashWrapper) {
    customWarning(table,      hashTableError::HASH_TABLE_BAD_POINTER);
    customWarning(testData,   hashTableError::TEXT_DATA_BAD_POINTER);

    for (size_t i = 0; i < testData->lineCount; i++) {
        string *data = createNode(testData->lineArray[i].linePointer, testData->lineArray[i].lineSize);
        customWarning(data, hashTableError::NODE_CREATION_ERROR);

        hashTableError error = searchString(table, data, hashWrapper);
        customWarning(error == hashTableError::NO_ERRORS, hashTableError::SEARCH_DATA_ERROR);

        destroyNode(data);
    }

    return hashTableError::NO_ERRORS;
}

#endif // BENCHMARK_H_

