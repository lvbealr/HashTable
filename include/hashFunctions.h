#ifndef HASH_FUNCTIONS_H_
#define HASH_FUNCTIONS_H_

#include <inttypes.h>
#include <stdlib.h>

#include "common.h"

inline uint32_t crc32(string *data) {
    char  *message = data->data;
    size_t length  = data->size;

    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++) {
        crc ^= (uint32_t)message[i];

        for (size_t j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }

    return crc ^ 0xFFFFFFFF;
}

#endif // HASH_FUNCTIONS_H_