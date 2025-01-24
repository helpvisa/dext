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

/* same as above but make sure a nullchar is inserted after the character */
void process_character_for_buffer_with_nullchar(Buffer* buffer, int buffer_index, char character, int insert_mode) {
    if (insert_mode) {
        insert_into_buffer(buffer, buffer_index, character);
        insert_into_buffer(buffer, buffer_index + 1, '\0');
    } else {
        replace_in_buffer(buffer, buffer_index, character);
    }
}

/* add a nullchar to the end of a given buffer, for safety */
void process_nullchar_for_buffer(Buffer* buffer, int buffer_index) {
    replace_in_buffer(buffer, buffer_index, '\0');
}

void delete_character_from_buffer(Buffer* buffer, int buffer_index) {
    int reverse_index = buffer_index;

    /* just make sure we don't access anything out of bounds */
    if (buffer_index < 0) {
        return;
    }
    while (reverse_index < buffer->allocated && buffer->content[reverse_index] != '\0') {
        buffer->content[reverse_index] = buffer->content[reverse_index + 1];
        reverse_index++;
    }
}

/* used to take characters from another buffer (usually the next line) and
 * bring them into the current buffer (i.e. we delete a newline character and
 * combine two lines into one line) */
void pilfer_character_from_buffer(Buffer* next_buffer, Buffer* current_buffer,
                                  int renderable_line_length) {
    int buffer_index = strlen(current_buffer->content);
    int check_index = buffer_index;
    char c = next_buffer->content[0];
    process_character_for_buffer(current_buffer, buffer_index, c, 1);
    delete_character_from_buffer(next_buffer, 0);
    /* determine if adding the next 'word' will overflow the line */
    while (check_index < current_buffer->allocated &&
           current_buffer->content[check_index] != ' ' &&
           current_buffer->content[check_index] != '\0') {
        check_index++;
    }
    /* and only add to this line if it doesn't */
    if (next_buffer->content[0] != '\0' && check_index < renderable_line_length) {
        pilfer_character_from_buffer(next_buffer, current_buffer, renderable_line_length);
    }
}

/* take all characters after the current buffer_index and pushb them to
 * the next_buffer */
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

/* travel to the beginning of the current word and bring all its characters
 * from the current_buffer into the next_buffer (useful when line-wrapping in
 * the middle of a word) */
void move_current_word_to_next_buffer(
         Buffer* next_buffer, Buffer* current_buffer,
         int buffer_index) {
    int break_index;
    int reverse_index = buffer_index;
    while (reverse_index > 0 && current_buffer->content[reverse_index] != ' ') {
        reverse_index--;
    }
    break_index = reverse_index;
    /* first we copy the word */
    while (reverse_index < current_buffer->allocated &&
           current_buffer->content[++reverse_index] != '\0') {
        process_character_for_buffer(next_buffer, strlen(next_buffer->content),
                                     current_buffer->content[reverse_index], 1);
    }
    /* now we delete the word */
    while (buffer_index > break_index) {
        delete_character_from_buffer(current_buffer, buffer_index);
        buffer_index--;
    }
}


void move_cursor_to_end_of_line(int* buffer_idx, int* line_idx, int insert,
                                int preferred_index,
                                struct Line* current_line) {
    int current_line_length = strlen(current_line->buffer->content) - 1;
    *buffer_idx = preferred_index;
    if (*buffer_idx > current_line_length) {
        *buffer_idx = current_line_length;
    }
    /* TODO: fix bug where cursor is placed on instead of after final character
     * for the last line of the document because this line's final character is
     * itself a nullchar */
    if (insert) {
        while (*buffer_idx > 0 &&
               current_line->buffer->content[*buffer_idx] == '\0') {
            *buffer_idx -= 1;
        }
    } else {
        while (*buffer_idx > 0 &&
               (current_line->buffer->content[*buffer_idx] == '\n' ||
               current_line->buffer->content[*buffer_idx] == '\0')) {
            *buffer_idx -= 1;
        }
    }
}

void move_cursor_down_line(
        int* buffer_idx, int* line_idx, int insert, int* preferred_index,
        struct Line* head, struct Line** current_line, int total_lines) {
    if (*line_idx + 1 < total_lines) {
        *line_idx += 1;
        *current_line = find_line_at_index(head, *line_idx);
        int current_line_length = strlen((*current_line)->buffer->content) - 1;
        *buffer_idx = *preferred_index;
        if (*buffer_idx > current_line_length) {
            move_cursor_to_end_of_line(buffer_idx, line_idx, insert,
                                       *preferred_index, *current_line);
        }
    } else {
        /* set to end of line */
        move_cursor_to_end_of_line(buffer_idx, line_idx, insert,
                                   9999, *current_line);
        *preferred_index = *buffer_idx;
    }
}

void move_cursor_up_line(
        int* buffer_idx, int* line_idx, int insert, int* preferred_index,
        struct Line* head, struct Line** current_line, int total_lines) {
    if (*line_idx > 0) {
        *line_idx -= 1;
        *current_line = find_line_at_index(head, *line_idx);
        int current_line_length = strlen((*current_line)->buffer->content) - 1;
        *buffer_idx = *preferred_index;
        if (*buffer_idx > current_line_length) {
            move_cursor_to_end_of_line(buffer_idx, line_idx, insert,
                                       *preferred_index, *current_line);
        }
    } else {
        /* reset cursor if we are already on first line */
        *buffer_idx = 0;
        *preferred_index = *buffer_idx;
    }
}
