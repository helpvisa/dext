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

int init_paragraph(struct Paragraph** new_paragraph);
int allocate_memory_to_paragraph(struct Paragraph** paragraph);
void free_paragraph(struct Paragraph** dead_paragraph);

int init_body(struct Body** new_body);
int allocate_memory_to_body(struct Body** body);
void free_body(struct Body** dead_body);

// parse data
int break_into_words(struct Line** line, char* buffer, int buff_size);
int organize_paragraph_content(struct Paragraph** paragraph, int line_length);
int break_into_paragraphs(struct Body** body, char* buffer, int buff_size);

#endif
