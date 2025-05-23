#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#include <fcntl.h>
#include <immintrin.h>
#include <unistd.h>

#include "IO.h"
#include "hashTable.h"
#include "hashFunctions.h"

const size_t MAX_LINE_SIZE = 256;

const size_t SEARCH_REPEAT_COUNT = 100000;

#define WARMING_UP(...) { __VA_ARGS__ }

#define TIME(...) do {                                               \
    uint64_t start = __rdtsc();                                      \
    __VA_ARGS__                                                      \
    uint64_t end   = __rdtsc();                                      \
                                                                     \
    uint64_t ticks = end - start;                                    \
    customPrint(purple, bold, bgDefault,                             \
    "(%s): %lu\n\n", hashFunctionName, ticks / SEARCH_REPEAT_COUNT); \
} while (0)

enum class benchmarkMode {
    BENCHMARK_MODE_WARMING_UP = 0,
    BENCHMARK_MODE_TIME       = 1
};

struct Dictionary {
    string **data = {};
    size_t   size = {};
};

hashTableError benchmarkHashTable(hashFunctionWrapper hashWrapper,
                                  const char *inputFile, const char *testFile, const char *outputFile,
                                  const char *hashFunctionName);

hashTableError saveHashTableToFile(hashTable<string *> *table, const char *outputFile);
hashTableError searchData         (hashTable<string *> *table, Dictionary *dictionary, hashFunctionWrapper hashWrapper);

hashTableError createDictionary(textData *testData, Dictionary *dictionary);

hashTableError benchmarkHashTable(hashFunctionWrapper hashWrapper,
                                  const char *inputFile, const char *testFile, const char *outputFile,
                                  const char *hashFunctionName, benchmarkMode mode) {
    customWarning(inputFile,  hashTableError::FILE_NOT_FOUND);
    customWarning(testFile,   hashTableError::FILE_NOT_FOUND);
    customWarning(outputFile, hashTableError::FILE_NOT_FOUND);

    textData inputData = {};
    textDataInitialize(inputFile, &inputData);

    textData testData = {};
    textDataInitialize(testFile, &testData);

    hashTable<string *> hashTable = {};
    initializeHashTable(&hashTable, 32768);

    fillHashTable(&hashTable, &inputData, hashWrapper);

    hashTableError saveStatus = saveHashTableToFile(&hashTable, outputFile);
    customWarning(saveStatus == hashTableError::NO_ERRORS, hashTableError::SAVE_HASH_TABLE_ERROR);

    Dictionary dictionary = {};
    hashTableError createStatus = createDictionary(&testData, &dictionary);
    customWarning(createStatus == hashTableError::NO_ERRORS, hashTableError::DICTIONARY_BAD_POINTER);

    switch (mode) {
        case benchmarkMode::BENCHMARK_MODE_WARMING_UP:
            WARMING_UP(
                for (size_t i = 0; i < SEARCH_REPEAT_COUNT; i++) {
                    customPrint(purple, bold, bgDefault, "\r(%s) Warming up [%lu/%lu]\t", hashFunctionName, i + 1, SEARCH_REPEAT_COUNT);
                    hashTableError searchStatus = searchData(&hashTable, &dictionary, hashWrapper);
                    customWarning(searchStatus == hashTableError::NO_ERRORS, hashTableError::SEARCH_DATA_ERROR);
                }
                customPrint(purple, bold, bgDefault, "\n(%s) Warming up done \t\n", hashFunctionName);
            );

            break;

        case benchmarkMode::BENCHMARK_MODE_TIME:
            TIME(
                customPrint(purple, bold, bgDefault, "(%s) Start benchmarking\n", hashFunctionName);
                for (size_t i = 0; i < SEARCH_REPEAT_COUNT; i++) {
                    customPrint(green, bold, bgDefault, "\r%lu / %lu", i, SEARCH_REPEAT_COUNT);
                    hashTableError searchStatus = searchData(&hashTable, &dictionary, hashWrapper);
                    customWarning(searchStatus == hashTableError::NO_ERRORS, hashTableError::SEARCH_DATA_ERROR);
                }
                customPrint(purple, bold, bgDefault, "(%s) Benchmarking done\n", hashFunctionName);
            );    

            break;

        default:
            customPrint(red, bold, bgDefault, "Unknown benchmark mode\n");
            break;
    }

    textDataDestruct(&inputData);
    textDataDestruct(&testData);

    // TODO: destroy dictionary

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

hashTableError createDictionary(textData *testData, Dictionary *dictionary) {
    customWarning(testData,   hashTableError::TEXT_DATA_BAD_POINTER);
    customWarning(dictionary, hashTableError::DICTIONARY_BAD_POINTER);

    dictionary->size = testData->lineCount;

    dictionary->data = (string **)calloc(dictionary->size, sizeof(string *));
    customWarning(dictionary->data, hashTableError::BUFFER_ERROR);
    
    string **dict = dictionary->data;

    for (size_t i = 0; i < testData->lineCount; i++) {
        string *data = createNode(testData->lineArray[i].linePointer, testData->lineArray[i].lineSize);
        customWarning(data, hashTableError::NODE_CREATION_ERROR);

        dict[i] = data;
    }

    return hashTableError::NO_ERRORS;
}

__attribute__((noinline)) hashTableError searchData(hashTable<string *> *table, Dictionary *dictionary, hashFunctionWrapper hashWrapper) {
    customWarning(table, hashTableError::HASH_TABLE_BAD_POINTER);

    string **dict = dictionary->data;

    for (size_t i = 0; i < dictionary->size; i++) {
        hashTableError error = searchString(table, dict[i], hashWrapper);
        customWarning(error == hashTableError::NO_ERRORS, hashTableError::SEARCH_DATA_ERROR);
    }

    return hashTableError::NO_ERRORS;
}

#endif // BENCHMARK_H_

