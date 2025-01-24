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
void process_character_for_buffer_with_nullchar(
    Buffer* buffer,
    int buffer_index,
    char character,
    int insert_mode
);
void process_nullchar_for_buffer(
    Buffer* buffer,
    int buffer_index
);
void delete_character_from_buffer(Buffer* buffer, int buffer_index);
void pilfer_character_from_buffer(
    Buffer* next_buffer,
    Buffer* current_buffer,
    int renderable_line_length
);
void push_to_next_buffer(Buffer* next_buffer, Buffer* current_buffer, int buffer_index);
void move_current_word_to_next_buffer(Buffer* next_buffer, Buffer* current_buffer, int buffer_index);

void move_cursor_to_end_of_line(
    int* buffer_idx, int* line_idx, int insert, int preferred_index,
    struct Line* current_line
);

void move_cursor_down_line(
    int* buffer_idx, int* line_idx, int insert, int* preferred_index,
    struct Line* head, struct Line** current_line, int total_lines
);

void move_cursor_up_line(
    int* buffer_idx, int* line_idx, int insert, int* preferred_index,
    struct Line* head, struct Line** current_line, int total_lines
);

#endif
