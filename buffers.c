#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "structs.h"

const int alloc_step = 128;

void free_buffer(Buffer **buffer) {
    if (*buffer) {
        if ((*buffer)->content) {
            free((*buffer)->content);
        }
        free(*buffer);
    }
}

uint64_t init_buffer(Buffer **buffer) {
    uint64_t to_allocate = alloc_step * sizeof(char);

    // re-initialize if buffer is not empty
    if (*buffer) {
        free_buffer(buffer);
    }

    *buffer = malloc(sizeof(**buffer));
    if (NULL == *buffer) {
        return -1;
    }
    (*buffer)->content = malloc(to_allocate);
    if (NULL == (*buffer)->content) {
        // we did not successfully allocate memory
        (*buffer)->allocated = 0;
        return -1;
    }
    (*buffer)->allocated = to_allocate;
    memset((*buffer)->content, 0, alloc_step);

    return to_allocate;
}

uint64_t expand_buffer(Buffer **buffer) {
    uint64_t new_size = (*buffer)->allocated + alloc_step;

    char* temp_allocation = realloc((*buffer)->content, new_size);
    if (NULL == temp_allocation) {
        return -1;
    }
    (*buffer)->content = temp_allocation;
    // zero our newly allocated memory chunk
    memset((*buffer)->content + (*buffer)->allocated, 0, alloc_step);
    (*buffer)->allocated = new_size;

    return new_size;
}

void add_line_to_beginning(struct Line **head, Buffer *buffer) {
    struct Line *new_line = malloc(sizeof(*new_line));
    new_line->buffer = buffer;
    new_line->next = *head;
    *head = new_line;
}

void add_line_to_end(struct Line **head, Buffer *buffer) {
    struct Line *new_line = malloc(sizeof(*new_line));
    new_line->buffer = buffer;
    new_line->next = NULL;

    if (NULL == *head) {
        *head = new_line;
        return;
    }

    struct Line *current_line = *head;
    while (NULL != current_line->next) {
        current_line = current_line->next;
    }
    current_line->next = new_line;
}

void insert_line(struct Line **ref, Buffer *buffer) {
    struct Line *new_line = malloc(sizeof(*new_line));
    new_line->buffer = buffer;
    new_line->next = (*ref)->next;
    (*ref)->next = new_line;
}

void remove_line(struct Line **head, struct Line **to_remove) {
    struct Line *current_line = *head, *previous_line;

    if (NULL != current_line && current_line->buffer == (*to_remove)->buffer) {
        *to_remove = current_line->next;
        *head = current_line->next;
        free_buffer(&current_line->buffer);
        free(current_line);
        return;
    }

    while (NULL != current_line && current_line->buffer != (*to_remove)->buffer) {
        previous_line = current_line;
        current_line = current_line->next;
    }

    if (NULL == current_line) {
        return;
    }

    previous_line->next = current_line->next;
    *to_remove = previous_line;
    free_buffer(&current_line->buffer);
    free(current_line);
}
