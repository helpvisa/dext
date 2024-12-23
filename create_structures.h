#ifndef CREATE_STRUCTURES_H_
#define CREATE_STRUCTURES_H_

#include "structs.h"

// initialize and free structures
int init_word(struct Word** new_word);
void free_word(struct Word** dead_word);
struct Line* init_line();
struct Paragraph* init_paragraph();

// parse data
int break_into_paragraphs(char* buffer);
int break_into_words(struct Word** words, char* buffer, int buff_size);

#endif
