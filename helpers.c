#include <stdlib.h>
#include "structs.h"
#include "buffers.h"

void insert_into_buffer(Buffer* buffer, int idx, char c) {
    // first, check if a reallocation is needed outright
    if (idx >= buffer->allocated) {
        expand_buffer(&buffer);
    }
    // walk forward thru buffer to make room for insertion
    int reverse_idx = idx;
    while (reverse_idx < buffer->allocated && buffer->content[reverse_idx] != '\0') {
        reverse_idx++;
    }
    // it is possible we need to reallocate if we are inserting b/w chars
    if (reverse_idx >= buffer->allocated) {
        expand_buffer(&buffer);
    }
    // move chars forward to make room b/w
    while (reverse_idx >= idx) {
        buffer->content[reverse_idx + 1] = buffer->content[reverse_idx];
        reverse_idx--;
    }
    // finally, we add the char
    buffer->content[idx] = c;
}

void replace_in_buffer(Buffer* buffer, int idx, char c) {
    if (idx >= buffer->allocated) {
        expand_buffer(&buffer);
    }
    buffer->content[idx] = c;
}

// if index is too large, will return last available non-null Line
struct Line* find_line_at_index(struct Line *head, int idx) {
    int current_line_index = 0;
    struct Line* current_line = head;
    while (current_line_index != idx && NULL != current_line->next) {
        current_line = current_line->next;
        current_line_index++;
    }
    return current_line;
}

void process_character_for_buffer(Buffer* buffer, int buffer_index, char character, int insert_mode) {
    if (insert_mode) {
        insert_into_buffer(buffer, buffer_index, character);
    } else {
        replace_in_buffer(buffer, buffer_index, character);
    }
}
