#include <stdlib.h>
#include <string.h>
#include "structs.h"


// initialize structures
struct Word* init_word() {
    struct Word* new_word = malloc(sizeof(*new_word));
    memset(new_word, 0, sizeof(struct Word));
    return new_word;
}

struct Line* init_line() {
    struct Line* new_line = malloc(sizeof(*new_line));
    memset(new_line, 0, sizeof(struct Line));
    return new_line;
}

struct Paragraph* init_paragraph() {
    struct Paragraph* new_paragraph = malloc(sizeof(*new_paragraph));
    memset(new_paragraph, 0, sizeof(struct Line));
    return new_paragraph;
}


int break_into_paragraphs(char** buffer) {
    int number_of_paragraphs = 0;
    return number_of_paragraphs;
}
