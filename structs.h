#ifndef STRUCTS_H_
#define STRUCTS_H_

#include <stdint.h>

/* structures for buffer and line editing */
typedef struct {
    char* content;
    uint64_t allocated;
} Buffer;

struct Line {
    Buffer *buffer;
    struct Line *next;
};

#endif
