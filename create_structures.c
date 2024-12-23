#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "structs.h"


// initialize and free structures
int init_word(struct Word** new_word) {
    *new_word = malloc(sizeof(**new_word));
    if (!*new_word) {
        return 1;
    }
    memset(*new_word, 0, sizeof(struct Word));

    (*new_word)->characters = malloc(16 * sizeof(char));
    if (!(*new_word)->characters) {
        return 1;
    }

    (*new_word)->size = 0;
    (*new_word)->allocated = 16;
    return 0;
}

int allocate_memory_to_word(struct Word** word) {
    int old_alloc_count = (*word)->allocated;
    int new_alloc_count = old_alloc_count + 16;
    char* new_allocation = NULL;
    new_allocation = realloc((*word)->characters,
                             new_alloc_count * sizeof(char));
    if (!new_allocation) {
        return 1;
    }

    (*word)->characters = new_allocation;
    (*word)->allocated = new_alloc_count;
    memset((*word)->characters + old_alloc_count, 0, 16);

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
    if (!(*new_line)) {
        return 1;
    }
    memset(*new_line, 0, sizeof(struct Line));

    /* (*new_line)->words = malloc(16 * sizeof(int*)); */
    (*new_line)->words = malloc(16 * sizeof(struct Word*));
    if (!(*new_line)->words) {
        return 1;
    }
    memset((*new_line)->words, 0, 16 * sizeof(struct Word*));

    (*new_line)->size = 0;
    (*new_line)->allocated = 16;

    return 0;
}

int allocate_memory_to_line(struct Line** line) {
    int old_alloc_count = (*line)->allocated;
    int new_alloc_count = old_alloc_count + 16;
    struct Word** new_allocation = NULL;
    new_allocation = realloc((*line)->words,
                             new_alloc_count * sizeof(struct Word*));
    if (!new_allocation) {
        return 1;
    }

    (*line)->words = new_allocation;
    (*line)->allocated = new_alloc_count;
    memset((*line)->words + old_alloc_count, 0, 16);

    return 0;
}

void free_line(struct Line** dead_line) {
    for (int i = 0; i < (*dead_line)->size; i++) {
        free_word(&(*dead_line)->words[i]);
        (*dead_line)->words[i] = NULL;
    }
    free((*dead_line)->words);
    (*dead_line)->words = NULL;
    free(*dead_line);
    *dead_line = NULL;
}

struct Paragraph* init_paragraph() {
    struct Paragraph* new_paragraph = malloc(sizeof(*new_paragraph));
    memset(new_paragraph, 0, sizeof(struct Line));
    return new_paragraph;
}


int break_into_words(struct Line** line, char* buffer, int buff_size) {
    int number_of_words = 0;
    int idx = 0;

    while (idx < buff_size && buffer[idx] != '\0' && buffer[idx] != EOF) {
        struct Word* new_word = NULL;
        init_word(&new_word);
        new_word->size = 0;

        while (buffer[idx] != ' ' &&
               buffer[idx] != '\n' &&
               buffer[idx] != '\t' &&
               buffer[idx] != EOF &&
               buffer[idx] != '\0') {
            new_word->characters[new_word->size] = buffer[idx];
            new_word->size++;
            if (new_word->size == new_word->allocated) {
                allocate_memory_to_word(&new_word);
            }
            idx++;
        }
        idx++; // move past the space

        new_word->characters[new_word->size] = '\0';
        (*line)->words[number_of_words] = new_word;
        number_of_words++;
        if (number_of_words == (*line)->allocated) {
            allocate_memory_to_line(line);
        }
    }

    return number_of_words;
}

int break_into_paragraphs(char* buffer) {
    int number_of_paragraphs = 0;
    return number_of_paragraphs;
}
