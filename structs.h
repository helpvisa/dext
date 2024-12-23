#ifndef STRUCTS_H_
#define STRUCTS_H_

struct Word {
    char* characters;
    int size;
    int allocated_chars;
};

struct Line {
    struct Word* words;
};

struct Paragraph {
    struct Line* lines;
};

#endif
