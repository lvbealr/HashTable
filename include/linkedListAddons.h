#ifndef LINKED_LIST_ADDONS_H_
#define LINKED_LIST_ADDONS_H_

#include <string.h>

#include "linkedList.h"

struct string {
    char  *data  = {};
    size_t size  = {};
};

string *createNode(const char *data) {
    string *newNode = (string *)calloc(1, sizeof(string));
    CHECK_FOR_NULL(newNode, return NULL);

    newNode->size = strlen(data);
    newNode->data = (char *)calloc(newNode->size + 1, sizeof(char));

    CHECK_FOR_NULL(newNode->data, FREE_(newNode); return NULL);

    strncpy(newNode->data, data, newNode->size);
    newNode->data[newNode->size] = '\0';

    return newNode;
}

linkedListError insertNode(linkedList<string *> *list, string *data) {
    customWarning(list,                linkedListError::LIST_BAD_POINTER);
    customWarning(list->freeNode != 0, linkedListError::BAD_CAPACITY);
    customWarning(data,                linkedListError::BAD_DATA_POINTER);

    ssize_t newNodeIndex = list->freeNode;

    list->freeNode = list->next[newNodeIndex];

    list->data[newNodeIndex] = data;

    ssize_t insertAfter = list->newIndex;

    if (insertAfter == 0) {
        if (list->next[0] == 0) {
            list->next[0] = newNodeIndex;
            list->prev[0] = newNodeIndex;

            list->next[newNodeIndex] = 0;
            list->prev[newNodeIndex] = 0;
        } else {
            list->next[newNodeIndex]  = list->next[0];
            list->prev[list->next[0]] = newNodeIndex;

            list->prev[newNodeIndex] = 0;
            list->next[0]            = newNodeIndex;
        }
    } else {
        list->next[newNodeIndex] = list->next[insertAfter];

        if (list->next[insertAfter] != 0) {
            list->prev[list->next[insertAfter]] = newNodeIndex;
        } else {
            list->prev[0] = newNodeIndex;
        }

        list->prev[newNodeIndex] = insertAfter;
        list->next[insertAfter] = newNodeIndex;
    }

    list->size++;
    list->newIndex = newNodeIndex;

    verifyLinkedList(list);

    return linkedListError::NO_ERRORS;
}

linkedListError deleteNode(linkedList<string *> *list, ssize_t index) {
    customWarning(list,                                linkedListError::LIST_BAD_POINTER);
    customWarning(index > 0 && index < list->capacity, linkedListError::BAD_INDEX);
    customWarning(list->prev[index] != -1,             linkedListError::BAD_INDEX);

    if (list->next[index] != 0) {
        list->prev[list->next[index]] = list->prev[index];
    } else {
        list->prev[0] = list->prev[index];
    }
    if (list->prev[index] != 0) {
        list->next[list->prev[index]] = list->next[index];
    } else {
        list->next[0] = list->next[index];
    }

    FREE_(list->data[index]);

    list->next[index] = list->freeNode;
    list->prev[index] = -1;
    list->freeNode    = index;

    list->size--;

    verifyLinkedList(list);

    return linkedListError::NO_ERRORS;
}

linkedListError getNodeValue(linkedList<string *> *list, ssize_t index, string **value) {
    customWarning(list,                                linkedListError::LIST_BAD_POINTER);
    customWarning(index > 0 && index < list->capacity, linkedListError::BAD_INDEX);
    customWarning(list->prev[index] != -1,             linkedListError::BAD_INDEX);
    customWarning(value,                               linkedListError::BAD_GET_NODE_POINTER);

    *value = list->data[index];

    verifyLinkedList(list);

    return linkedListError::NO_ERRORS;
}

#endif // LINKED_LIST_ADDONS_H_