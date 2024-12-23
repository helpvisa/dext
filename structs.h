#ifndef STRUCTS_H_
#define STRUCTS_H_

// generic structures
struct Word {
    char* characters;
    int size;
    int allocated;
};

struct Line {
    struct Word** words;
    int size;
    int allocated;
};

struct Paragraph {
    struct Line** lines;
    int size;
    int allocated;
};

#endif
