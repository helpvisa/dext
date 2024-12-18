#ifndef STRUCTS_H_
#define STRUCTS_H_

struct Word {
    char* characters;
};

struct Line {
    struct Word* words;
};

struct Paragraph {
    struct Line* lines;
};

#endif
