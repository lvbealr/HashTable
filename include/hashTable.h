#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include <inttypes.h>
#include <stdlib.h>

#include "common.h"
#include "hashFunctions.h"
#include "linkedList.h"
#include "linkedListAddons.h"

const double   MAX_LOAD_FACTOR = 15.0f;

template<typename T>
struct hashTable {
    linkedList<T> *table = {};

    size_t size     = {};
    size_t capacity = {};

    double maxLoadFactor = MAX_LOAD_FACTOR;
};

enum class hashTableError {
    NO_ERRORS               = 0,
    HASH_TABLE_BAD_POINTER  = 1,
    NODE_CREATION_ERROR     = 2,
    NODE_INSERTION_ERROR    = 3,
    HASH_TABLE_FILL_ERROR   = 4,
};

template<typename T>
hashTableError initializeHashTable(hashTable<T> *table, size_t capacity) {
    customWarning(table, hashTableError::HASH_TABLE_BAD_POINTER);

    table->capacity = capacity;
    table->size     = 0;

    table->table = (linkedList<T> *)calloc(table->capacity, sizeof(linkedList<T>));
    customWarning(table->table, hashTableError::HASH_TABLE_BAD_POINTER);

    for (size_t i = 0; i < table->capacity; i++) {
        initializeLinkedList<T>(&table->table[i], 8);
    }

    return hashTableError::NO_ERRORS;
}

template<typename T>
hashTableError destroyHashTable(hashTable<T> *table) {
    customWarning(table, hashTableError::HASH_TABLE_BAD_POINTER);

    for (size_t i = 0; i < table->capacity; i++) {
        linkedList<T> *list    = &table->table[i];
        ssize_t        current = list->next[0];

        while (current != 0) {
            T value = list->data[current];

            if (value) {
                destroyNode(value);
                list->data[current] = NULL;
            }

            current = list->next[current];
        }

        destroyLinkedList(list);
    }

    FREE_(table->table);

    return hashTableError::NO_ERRORS;
}

template<typename T>
hashTableError fillHashTableWithRehash(hashTable<T> *table, textData *data, hashFunctionWrapper hashWrapper) {
    hashTableError error = fillHashTable(table, data, hashWrapper);
    customWarning(error == hashTableError::NO_ERRORS, hashTableError::HASH_TABLE_FILL_ERROR);

    double currentLoadFactor = double(table->size) / double(table->capacity);

    while (currentLoadFactor > table->maxLoadFactor) {
        error = rehashTable(table, hashWrapper);

        if (error != hashTableError::NO_ERRORS) {
            return error;
        }

        currentLoadFactor = double(table->size) / double(table->capacity);
    }

    return hashTableError::NO_ERRORS;
}

template<typename T>
hashTableError fillHashTable(hashTable<T> *table, textData *data, hashFunctionWrapper hashWrapper) {
    for (size_t line = 0; line < data->lineCount; line++) {
        T newWord = createNode(data->lineArray[line].linePointer, data->lineArray[line].lineSize);
        customWarning(newWord, hashTableError::NODE_CREATION_ERROR);

        uint32_t hashValue = hashWrapper(newWord, SEED) % table->capacity;

        linkedListError insertError = insertNode(&table->table[hashValue], newWord);

        if (insertError != linkedListError::NO_ERRORS) {
            destroyNode(newWord);
            return hashTableError::NODE_INSERTION_ERROR;
        }

        table->size++;
    }

    return hashTableError::NO_ERRORS;
}

template<typename T>
hashTableError rehashTable(hashTable<T> *table, hashFunctionWrapper hashWrapper) {
    size_t newCapacity = table->capacity * 2;

    hashTable<T> newTable = {};

    hashTableError initError = initializeHashTable(&newTable, newCapacity);
    if (initError != hashTableError::NO_ERRORS) {
        return initError;
    }

    for (size_t i = 0; i < table->capacity; i++) {
        linkedList<T> *list    = &table->table[i];
        ssize_t        current = list->next[0];

        while (current != 0) {
            T value = list->data[current];

            if (value) {
                uint32_t newHash = hashWrapper(value, SEED) % newCapacity;

                linkedListError insertError = insertNode(&newTable.table[newHash], value);

                if (insertError != linkedListError::NO_ERRORS) {
                    destroyNode(value);
                } else {
                    list->data[current] = NULL;
                    newTable.size++;
                }
            }

            current = list->next[current];
        }
    }

    destroyHashTable(table);
    *table = newTable;

    return hashTableError::NO_ERRORS;
}

#endif // HASH_TABLE_H_