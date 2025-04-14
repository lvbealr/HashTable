#ifndef IO_H_
#define IO_H_

#include <cstdio>

enum class IOError {
    FILE_NOT_FOUND        = -1,
    NO_ERRORS             =  0,
    TEXT_DATA_BAD_POINTER =  1,
    TEXT_LINE_BAD_POINTER =  2,
    READING_ERROR         =  3,
    BAD_FILE_NAME         =  4,
};

struct textLine {
    char  *linePointer  = {};
    size_t lineSize     = {};
};

struct textData {
    ssize_t   fileSize  = {};
    size_t    lineCount = {};
    char     *text      = {};
    textLine *lineArray = {};
};

#define FREE_(ptr) do { \
    free(ptr);          \
    ptr = NULL;         \
} while (0)

IOError textDataInitialize(const char *fileName, textData *textData);
IOError textDataDestruct  (textData *textData);

size_t  lineCounter       (textData *textData);

IOError textLinePoint     (textData *textData, textLine *lineArray);
IOError textLineInitialize(textData *textData, textLine *lineArray);
IOError textLineDestruct  (textLine *lineArray);

#endif // IO_H_