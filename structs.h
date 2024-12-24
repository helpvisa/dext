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
    struct Line* content;
    struct Line** formatted_lines;
    int size;
    int allocated;
};

struct Body {
    struct Paragraph** paragraphs;
    int size;
    int allocated;
};

// structures for buffer and line editing
struct Buffer {
    char* content;
    int allocated;
};

struct LineBuffer {
    struct Buffer** buffers;
    int allocated;
};

#endif
