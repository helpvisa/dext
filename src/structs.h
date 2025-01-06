#ifndef STRUCTS_H_
#define STRUCTS_H_

/* structures for buffer and line editing */
typedef struct {
    char* content;
    unsigned int allocated;
} Buffer;

struct Line {
    Buffer *buffer;
    struct Line *next;
};

#endif
