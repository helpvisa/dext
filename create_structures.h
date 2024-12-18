#ifndef CREATE_STRUCTURES_H_
#define CREATE_STRUCTURES_H_

#include "structs.h"

// initialize structures
struct Word* init_word();
struct Line* init_line();
struct Paragraph* init_paragraph();

// parse data
int break_into_paragraphs(char** buffer);

#endif
