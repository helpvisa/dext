#ifndef CREATE_STRUCTURES_H_
#define CREATE_STRUCTURES_H_

#include "structs.h"

// initialize and free structures
int init_word(struct Word** new_word);
int allocate_memory_to_word(struct Word** word);
void free_word(struct Word** dead_word);
int init_line(struct Line** new_line);
int allocate_memory_to_line(struct Line** line);
void free_line(struct Line** dead_line);
struct Paragraph* init_paragraph();

// parse data
int break_into_paragraphs(char* buffer);
int break_into_words(struct Line** line, char* buffer, int buff_size);

#endif
