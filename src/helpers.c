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
        int cx, int cy, int left_margin,
        int* buffer_idx, int* line_idx, int renderable_line_length,
        struct Line* head, struct Line** current_line, int total_lines) {
    int local_cx, local_cy, line_counter = 0;
    int current_line_idx = 0;
    char* content = (*current_line)->buffer->content;
    int content_length = strlen(content);

    local_cy = *line_idx * 2;
    while (line_counter < *line_idx) {
        Buffer* count_buffer = find_line_at_index(head, line_counter)->buffer;
        local_cy += (strlen(count_buffer->content) - 1) / renderable_line_length;
        line_counter++;
    }

    local_cx = left_margin;
    while(current_line_idx < content_length) {
        int current_word_length = 0;
        while (current_line_idx < content_length &&
               ' ' != content[current_line_idx] &&
               '\n' != content[current_line_idx] &&
               '\0' != content[current_line_idx]) {
            if (local_cy == cy + 1 && local_cx == cx) {
                *buffer_idx = current_line_idx;
                return;
            }
            current_word_length++;
            current_line_idx++;
            local_cx += 1;
        }
        /* go back to beginning of word so we don't miss indices on new line */
        if (local_cx - left_margin > renderable_line_length - 1) {
            local_cx = left_margin;
            current_line_idx -= (current_word_length);
            local_cy += 1;
        }
        if (local_cy == cy + 1 && local_cx == cx) {
            *buffer_idx = current_line_idx;
            return;
        }
        local_cx += 1;
        current_line_idx++;
    }
    /* we're still here? we need to go down a line */
    if (*line_idx < total_lines - 1) {
        *line_idx += 1;
        *current_line = find_line_at_index(head, *line_idx);
        content = (*current_line)->buffer->content;
        *buffer_idx = cx - left_margin;
        content_length = strlen(content);
        if (content_length < *buffer_idx) {
            *buffer_idx = content_length;
            if (*buffer_idx > 0 && content[*buffer_idx] == '\n') {
                *buffer_idx -= 1;
            }
        }
        return;
    } else {
        *buffer_idx = content_length;
        if (*buffer_idx > 0 && content[*buffer_idx] == '\n') {
            *buffer_idx -= 1;
        }
        return;
    }
}

void move_cursor_up_formatted_line(
        int cx, int cy, int left_margin,
        int* buffer_idx, int* line_idx, int renderable_line_length,
        struct Line* head, struct Line** current_line, int total_lines) {
    int local_cx, local_cy, line_counter = 0;
    int current_line_idx = 0;
    char* content = (*current_line)->buffer->content;
    int content_length = strlen(content);

    local_cy = *line_idx * 2;
    while (line_counter < *line_idx) {
        Buffer* count_buffer = find_line_at_index(head, line_counter)->buffer;
        local_cy += (strlen(count_buffer->content) - 1) / renderable_line_length;
        line_counter++;
    }

    local_cx = left_margin;
    while(current_line_idx < content_length) {
        int current_word_length = 0;
        while (current_line_idx < content_length &&
               ' ' != content[current_line_idx] &&
               '\n' != content[current_line_idx] &&
               '\0' != content[current_line_idx]) {
            if (local_cy == cy - 1 && local_cx == cx) {
                *buffer_idx = current_line_idx;
                return;
            }

            current_word_length++;
            current_line_idx++;
            local_cx += 1;
        }
        if (local_cx - left_margin > renderable_line_length - 1) {
            local_cx = left_margin;
            current_line_idx -= (current_word_length);
            local_cy += 1;
        }
        if (local_cy == cy - 1 && local_cx == cx) {
            *buffer_idx = current_line_idx;
            return;
        }
        local_cx += 1;
        current_line_idx++;
    }
    /* we're still here? we need to go up a line */
    if (*line_idx > 0) {
        int i;
        *line_idx -= 1;
        *current_line = find_line_at_index(head, *line_idx);
        content = (*current_line)->buffer->content;
        content_length = strlen(content);
        if (content_length < *buffer_idx) {
            *buffer_idx = content_length;
            if (*buffer_idx > 0 && content[*buffer_idx] == '\n') {
                *buffer_idx -= 1;
            }
        }
        /* call 'movedown' func for however many formatted
         * rows are left in the above line */
        /* for (i = 0; i < strlen(content) / renderable_line_length; i++) { */
        /*     move_cursor_down_formatted_line( */
        /*         cx, cy, left_margin, */
        /*         buffer_idx, line_idx, renderable_line_length, */
        /*         head, current_line, total_lines */
        /*     ); */
        /* } */
    } else {
        *buffer_idx = 0;
    }
}
