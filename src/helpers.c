#include "helpers.h"
#include <stdlib.h>
#include "structs.h"
#include "buffers.h"
#include "string.h"

void insert_into_buffer(Buffer* buffer, int idx, char c) {
    int reverse_idx = idx;

    /* first, check if a reallocation is needed outright */
    if (idx >= buffer->allocated - 1) {
        expand_buffer(&buffer);
    }
    /* walk forward thru buffer to make room for insertion */
    while (reverse_idx < buffer->allocated && buffer->content[reverse_idx] != '\0') {
        reverse_idx++;
    }
    /* it is possible we need to reallocate if we are inserting b/w chars */
    if (reverse_idx >= buffer->allocated - 1) {
        expand_buffer(&buffer);
    }
    /* move chars forward to make room b/w */
    while (reverse_idx >= idx) {
        buffer->content[reverse_idx + 1] = buffer->content[reverse_idx];
        reverse_idx--;
    }
    /* finally, we add the char */
    buffer->content[idx] = c;
}

void replace_in_buffer(Buffer* buffer, int idx, char c) {
    if (idx >= buffer->allocated - 1) {
        expand_buffer(&buffer);
    }
    buffer->content[idx] = c;
}

/* if index is too large, will return last available non-null Line */
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

void process_character_for_buffer_with_nullchar(Buffer* buffer, int buffer_index, char character, int insert_mode) {
    if (insert_mode) {
        insert_into_buffer(buffer, buffer_index, character);
        insert_into_buffer(buffer, buffer_index + 1, '\0');
    } else {
        replace_in_buffer(buffer, buffer_index, character);
    }
}

void process_nullchar_for_buffer(Buffer* buffer, int buffer_index) {
    replace_in_buffer(buffer, buffer_index, '\0');
}

void delete_character_from_buffer(Buffer* buffer, int buffer_index) {
    int reverse_index = buffer_index;

    /* just make sure we don't access anything oob */
    if (buffer_index < 0) {
        return;
    }
    while (reverse_index < buffer->allocated && buffer->content[reverse_index] != '\0') {
        buffer->content[reverse_index] = buffer->content[reverse_index + 1];
        reverse_index++;
    }
}

void pilfer_character_from_buffer(Buffer* next_buffer, Buffer* current_buffer) {
    int buffer_index = strlen(current_buffer->content) - 1;
    char c = next_buffer->content[0];
    process_character_for_buffer(current_buffer, buffer_index, c, 1);
    delete_character_from_buffer(next_buffer, 0);
    /* if (next_buffer->content[0] != '\n' && next_buffer->content[0] != '\0') { */
    if (next_buffer->content[0] != '\0') {
        pilfer_character_from_buffer(next_buffer, current_buffer);
    }
}

void push_to_next_buffer(Buffer* next_buffer, Buffer* current_buffer, int buffer_index) {
    int reverse_index = buffer_index;
    while (reverse_index < current_buffer->allocated && current_buffer->content[reverse_index] != '\0') {
        reverse_index++;
    }
    while (current_buffer->content[--reverse_index] != '\0' && reverse_index >= buffer_index) {
        process_character_for_buffer(next_buffer, 0, current_buffer->content[reverse_index], 1);
        delete_character_from_buffer(current_buffer, reverse_index);
    }
}

/* TODO: how many times the current line / buffer has been wrapped when rendered */
/* use in move_cursor_up_formatted_line to fix weird when changing line_idx */
void calculate_line_break_count() {
    ;
}

void move_cursor_down_formatted_line(
        int* buffer_idx, int* line_idx, int insert,
        struct Line* head, struct Line** current_line, int total_lines) {
    if (*line_idx + 1 < total_lines) {
        *line_idx += 1;
        *current_line = find_line_at_index(head, *line_idx);
        int current_line_length = strlen((*current_line)->buffer->content) - 1;
        if (*buffer_idx > current_line_length) {
            *buffer_idx = current_line_length;
            if (insert) {
                while (*buffer_idx > 0 &&
                       (*current_line)->buffer->content[*buffer_idx] == '\0') {
                    *buffer_idx -= 1;
                }
            } else {
                while (*buffer_idx > 0 &&
                       ((*current_line)->buffer->content[*buffer_idx] == '\n' ||
                       (*current_line)->buffer->content[*buffer_idx] == '\0')) {
                    *buffer_idx -= 1;
                }
            }
        }
    }
}

void move_cursor_up_formatted_line(
        int* buffer_idx, int* line_idx, int insert,
        struct Line* head, struct Line** current_line, int total_lines) {
    if (*line_idx - 1 > 0) {
        *line_idx -= 1;
        *current_line = find_line_at_index(head, *line_idx);
        int current_line_length = strlen((*current_line)->buffer->content) - 1;
        if (*buffer_idx > current_line_length) {
            *buffer_idx = current_line_length;
            if (insert) {
                while (*buffer_idx > 0 &&
                       (*current_line)->buffer->content[*buffer_idx] == '\0') {
                    *buffer_idx -= 1;
                }
            } else {
                while (*buffer_idx > 0 &&
                       ((*current_line)->buffer->content[*buffer_idx] == '\n' ||
                       (*current_line)->buffer->content[*buffer_idx] == '\0')) {
                    *buffer_idx -= 1;
                }
            }
        }
    }
}
