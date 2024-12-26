#ifndef HELPERS_H_
#define HELPERS_H_

#include "structs.h"

void insert_into_buffer(Buffer* buffer, int idx, char c);
void replace_in_buffer(Buffer* buffer, int idx, char c);

struct Line* find_line_at_index(struct Line *head, int idx);

void process_character_for_buffer(
    Buffer* buffer,
    int buffer_index,
    char character,
    int insert_mode
);

#endif
