#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "IO.h"
#include "customWarning.h"

IOError textLinePoint(textData *textData, textLine *lineArray) {
    customWarning(textData,  IOError::TEXT_DATA_BAD_POINTER);
    customWarning(lineArray, IOError::TEXT_LINE_BAD_POINTER);

    char *textPointer = textData->text;

    size_t lineIndex = 0;
    lineArray[lineIndex].linePointer = textPointer;

    for (size_t i = 0; i < (size_t)textData->fileSize; ++i) {
        if (*textPointer == '\n') {
            lineArray[lineIndex].lineSize = textPointer - lineArray[lineIndex].linePointer;
            lineIndex++;

            lineArray[lineIndex].linePointer = textPointer + 1;
        }

        textPointer++;
    }

    lineArray[lineIndex].lineSize = textPointer - lineArray[lineIndex].linePointer;

    return IOError::NO_ERRORS;
}

IOError textLineInitialize(textData *textData, textLine *lineArray) {
    customWarning(textData,  IOError::TEXT_DATA_BAD_POINTER);
    customWarning(lineArray, IOError::TEXT_LINE_BAD_POINTER);

    return textLinePoint(textData, lineArray);
}

IOError textLineDestruct(textLine *lineArray) {
    customWarning(lineArray, IOError::TEXT_LINE_BAD_POINTER);

    FREE_(lineArray);

    return IOError::NO_ERRORS;
}

IOError textDataInitialize(const char *fileName, textData *textData) {
    customWarning(fileName, IOError::BAD_FILE_NAME);
    customWarning(textData, IOError::TEXT_DATA_BAD_POINTER);

    struct stat fileData = {};
    
    if (stat(fileName, &fileData) != 0)
        return IOError::FILE_NOT_FOUND;

    textData->fileSize = (ssize_t)fileData.st_size;
    customWarning(textData->fileSize != 0, IOError::READING_ERROR);

    textData->text = (char *)calloc((size_t)textData->fileSize + 1, 1);

    int openFile = open(fileName, O_RDONLY);

    if (openFile == (int)IOError::FILE_NOT_FOUND) {
        FREE_(textData->text);
        return IOError::FILE_NOT_FOUND;
    }

    ssize_t sizeRead = read(openFile, textData->text, (size_t)textData->fileSize);
    close(openFile);

    if (sizeRead != textData->fileSize) {
        FREE_(textData->text);
        return IOError::READING_ERROR;
    }

    textData->lineCount = lineCounter(textData);
    textData->lineArray = (textLine *)calloc(textData->lineCount, sizeof(textLine));

    return textLineInitialize(textData, textData->lineArray);
}

IOError textDataDestruct(textData *textData) {
    customWarning(textData, IOError::TEXT_DATA_BAD_POINTER);

    FREE_(textData->text);
    FREE_(textData->lineArray);

    textData->fileSize  = 0;
    textData->lineCount = 0;

    return IOError::NO_ERRORS;
}

size_t lineCounter(textData *textData) {
    if (!textData || !textData->text || textData->fileSize <= 0)
        return 0;

    size_t lineCount = 0;

    char *ptr = textData->text;
    char *end = ptr + textData->fileSize;

    while (ptr < end) {
        if (*ptr++ == '\n')
            lineCount++;
    }

    if (textData->fileSize > 0 && textData->text[textData->fileSize - 1] != '\n') {
        lineCount++;
    }

    return lineCount;
}