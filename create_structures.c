#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "structs.h"
#include "helpers.h"

#define ALLOC_STEP 64

// initialize and free structures
int init_word(struct Word** new_word) {
    *new_word = malloc(sizeof(**new_word));
    if (NULL == *new_word) {
        return -1;
    }
    memset(*new_word, 0, sizeof(**new_word));

    (*new_word)->characters = malloc(ALLOC_STEP * sizeof(char));
    if (NULL == (*new_word)->characters) {
        return -1;
    }
    memset((*new_word)->characters, 0, ALLOC_STEP * sizeof(char));

    (*new_word)->size = 0;
    (*new_word)->allocated = ALLOC_STEP;
    return 0;
}

int allocate_memory_to_word(struct Word** word) {
    int old_alloc_count = (*word)->allocated;
    int new_alloc_count = old_alloc_count + ALLOC_STEP;
    char* new_allocation = NULL;
    new_allocation = realloc((*word)->characters,
                             new_alloc_count * sizeof(char));
    if (NULL == new_allocation) {
        return -1;
    }

    (*word)->characters = new_allocation;
    (*word)->allocated = new_alloc_count;
    memset((*word)->characters + old_alloc_count, 0, ALLOC_STEP);

    return 0;
}

void free_word(struct Word** dead_word) {
    free((*dead_word)->characters);
    (*dead_word)->characters = NULL;
    free(*dead_word);
    *dead_word = NULL;
}

int init_line(struct Line** new_line) {
    *new_line = malloc(sizeof(**new_line));
    if (NULL == *new_line) {
        return -1;
    }
    memset(*new_line, 0, sizeof(**new_line));

    (*new_line)->words = malloc(ALLOC_STEP * sizeof(struct Word*));
    if (NULL == (*new_line)->words) {
        return -1;
    }
    memset((*new_line)->words, 0, ALLOC_STEP * sizeof(struct Word*));

    (*new_line)->size = 0;
    (*new_line)->allocated = ALLOC_STEP;

    return 0;
}

int allocate_memory_to_line(struct Line** line) {
    int old_alloc_count = (*line)->allocated;
    int new_alloc_count = old_alloc_count + ALLOC_STEP;
    struct Word** new_allocation = NULL;
    new_allocation = realloc((*line)->words,
                             new_alloc_count * sizeof(struct Word*));
    if (NULL == new_allocation) {
        return -1;
    }

    (*line)->words = new_allocation;
    (*line)->allocated = new_alloc_count;
    memset((*line)->words + old_alloc_count, 0, ALLOC_STEP);

    return 0;
}

void free_line(struct Line** dead_line) {
    for (int i = 0; i < (*dead_line)->size; i++) {
        free_word(&(*dead_line)->words[i]);
        free((*dead_line)->words[i]);
        (*dead_line)->words[i] = NULL;
    }
    free((*dead_line)->words);
    (*dead_line)->words = NULL;
    free(*dead_line);
    *dead_line = NULL;
}

int init_paragraph(struct Paragraph** new_paragraph) {
    *new_paragraph = malloc(sizeof(**new_paragraph));
    if (NULL == *new_paragraph) {
        return -1;
    }
    memset(*new_paragraph, 0, sizeof(**new_paragraph));

    (*new_paragraph)->formatted_lines = malloc(ALLOC_STEP * sizeof(struct Line*));
    if (NULL == (*new_paragraph)->formatted_lines) {
        return -1;
    }
    memset((*new_paragraph)->formatted_lines, 0, ALLOC_STEP * sizeof(struct Line*));

    (*new_paragraph)->size = 0;
    (*new_paragraph)->allocated = ALLOC_STEP;
    (*new_paragraph)->content = NULL;
    init_line(&(*new_paragraph)->content);

    return 0;
}

int allocate_memory_to_paragraph(struct Paragraph** paragraph) {
    int old_alloc_count = (*paragraph)->allocated;
    int new_alloc_count = old_alloc_count + ALLOC_STEP;
    struct Line** new_allocation = NULL;
    new_allocation = realloc((*paragraph)->formatted_lines,
                             new_alloc_count * sizeof(struct Line*));
    if (NULL == new_allocation) {
        return -1;
    }

    (*paragraph)->formatted_lines = new_allocation;
    (*paragraph)->allocated = new_alloc_count;
    memset((*paragraph)->formatted_lines + old_alloc_count, 0, ALLOC_STEP);

    return 0;
}

void free_paragraph(struct Paragraph** dead_paragraph) {
    for (int i = 0; i < (*dead_paragraph)->size; i++) {
        free_line(&(*dead_paragraph)->formatted_lines[i]);
        free((*dead_paragraph)->formatted_lines[i]);
        (*dead_paragraph)->formatted_lines[i] = NULL;
    }
    free((*dead_paragraph)->formatted_lines);
    (*dead_paragraph)->formatted_lines = NULL;
    if ((*dead_paragraph)->content) {
        // we do not need to call free_line here; they have already been freed
        // by the above loop since their pointers are shared
        free((*dead_paragraph)->content);
        (*dead_paragraph)->content = NULL;
    }
    free(*dead_paragraph);
    *dead_paragraph = NULL;
}

int init_body(struct Body** new_body) {
    *new_body = malloc(sizeof(**new_body));
    if (NULL == *new_body) {
        return -1;
    }
    memset(*new_body, 0, sizeof(**new_body));

    (*new_body)->paragraphs = malloc(ALLOC_STEP * sizeof(struct Paragraph*));
    if (NULL == (*new_body)->paragraphs) {
        return -1;
    }
    memset((*new_body)->paragraphs, 0, ALLOC_STEP * sizeof(struct Paragraph*));

    (*new_body)->size = 0;
    (*new_body)->allocated = ALLOC_STEP;

    return 0;
}

int allocate_memory_to_body(struct Body** body) {
    int old_alloc_count = (*body)->allocated;
    int new_alloc_count = old_alloc_count + ALLOC_STEP;
    struct Paragraph** new_allocation = NULL;
    new_allocation = realloc((*body)->paragraphs,
                             new_alloc_count * sizeof(struct Paragraph*));
    if (NULL == new_allocation) {
        return -1;
    }

    (*body)->paragraphs = new_allocation;
    (*body)->allocated = new_alloc_count;
    memset((*body)->paragraphs + old_alloc_count, 0, ALLOC_STEP);

    return 0;
}

void free_body(struct Body** dead_body) {
    for (int i = 0; i < (*dead_body)->size; i++) {
        free_paragraph(&(*dead_body)->paragraphs[i]);
        (*dead_body)->paragraphs[i] = NULL;
    }
    free((*dead_body)->paragraphs);
    (*dead_body)->paragraphs = NULL;
    free(*dead_body);
    *dead_body = NULL;
}


// functions for splitting apart buffers
int break_into_words(struct Line** line, char* buffer, int buff_size) {
    // abort if line is null
    if (NULL == line || NULL == (*line)) {
        return -1;
    }

    int idx = 0;

    while (idx < buff_size && buffer[idx] != '\0') {
        struct Word* new_word = NULL;
        init_word(&new_word);

        while (buffer[idx] != ' ' &&
               buffer[idx] != '\n' &&
               buffer[idx] != '\t' &&
               buffer[idx] != '\0') {
            new_word->characters[new_word->size] = buffer[idx];
            new_word->size++;
            if (new_word->size >= new_word->allocated) {
                allocate_memory_to_word(&new_word);
            }
            idx++;
        }
        idx++; // move past the splitting character

        new_word->characters[new_word->size] = '\0';
        (*line)->words[(*line)->size] = new_word;
        (*line)->size++;
        if ((*line)->size >= (*line)->allocated) {
            allocate_memory_to_line(line);
        }
    }

    return (*line)->size;
}

int organize_paragraph_content(struct Paragraph** paragraph, int line_length) {
    int idx = 0;

    if (NULL == (*paragraph)->content) {
        return -1;
    }

    while (idx < (*paragraph)->content->size) {
        int curr_line_length = 0;
        struct Line* new_line = NULL;
        init_line(&new_line);

        int line_overflowed = 0;
        while (!line_overflowed) {
            if (NULL == (*paragraph)->content->words[idx]) {
                break;
            }

            if ((*paragraph)->content->words[idx]->size + curr_line_length <= line_length) {
                curr_line_length += (*paragraph)->content->words[idx]->size;
                new_line->words[new_line->size] = (*paragraph)->content->words[idx];
                new_line->size++;
                if (new_line->size >= new_line->allocated) {
                    allocate_memory_to_line(&new_line);
                }
                idx++; 
            } else {
                line_overflowed = 1;
            }
        }

        if (new_line->size > 0) {
            (*paragraph)->formatted_lines[(*paragraph)->size] = new_line;
            (*paragraph)->size++;
        } else {
            free_line(&new_line);
        }
    }

    return (*paragraph)->size;
}

int break_into_paragraphs(struct Body** body, char* buffer, int buff_size) {
    int idx = 0;
    char* temp_buffer = NULL;
    temp_buffer = malloc(ALLOC_STEP * sizeof(char));
    if (NULL == temp_buffer) {
        return -1;
    }
    int allocated_to_temp_buffer = ALLOC_STEP;

    while (idx < buff_size && buffer[idx] != '\0') {
        int temp_buffer_idx = 0;
        memset(temp_buffer, 0, allocated_to_temp_buffer);

        // don't count one-off newlines as their own paragraphs
        if (temp_buffer_idx == 0 && buffer[idx] == '\n') {
            idx++;
            continue;
        }

        struct Paragraph* new_paragraph = NULL;
        init_paragraph(&new_paragraph);

        while (idx < buff_size && buffer[idx] != '\n') {
            temp_buffer[temp_buffer_idx] = buffer[idx];
            temp_buffer_idx++;
            idx++;

            if (temp_buffer_idx >= allocated_to_temp_buffer) {
                allocated_to_temp_buffer = expand_buffer(&temp_buffer,
                                                         allocated_to_temp_buffer,
                                                         ALLOC_STEP);
                if (allocated_to_temp_buffer < 0) {
                    return -1;
                }
            }
        }
        // skip the newline, since it's been seen now
        idx++;
        
        break_into_words(&new_paragraph->content,
                         temp_buffer,
                         allocated_to_temp_buffer);
        organize_paragraph_content(&new_paragraph, 72);
        (*body)->paragraphs[(*body)->size] = new_paragraph;
        (*body)->size++;
        if ((*body)->size >= (*body)->allocated) {
            allocate_memory_to_body(body);
        }
    }

    free(temp_buffer);
    return (*body)->size;
}
