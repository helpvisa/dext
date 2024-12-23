#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "structs.h"


// initialize and free structures
int init_word(struct Word** new_word) {
    *new_word = malloc(sizeof(**new_word));
    memset(*new_word, 0, sizeof(struct Word));
    if (!*new_word) {
        return 1;
    }
    (*new_word)->characters = malloc(16 * sizeof(char));
    if (!(*new_word)->characters) {
        return 1;
    }
    (*new_word)->size = 0;
    (*new_word)->allocated_chars = 16;
    return 0;
}
void free_word(struct Word** dead_word) {
    free((*dead_word)->characters);
    (*dead_word)->characters = NULL;
    free((*dead_word));
    (*dead_word) = NULL;
}

struct Line* init_line() {
    struct Line* new_line = malloc(sizeof(*new_line));
    memset(new_line, 0, sizeof(struct Line));
    return new_line;
}

struct Paragraph* init_paragraph() {
    struct Paragraph* new_paragraph = malloc(sizeof(*new_paragraph));
    memset(new_paragraph, 0, sizeof(struct Line));
    return new_paragraph;
}


int break_into_words(struct Word** words, char* buffer, int buff_size) {
    int number_of_words = 0;
    int idx = 0;
    // TODO: create linked-list for words and move this declaration inside loop
    struct Word* new_word = NULL;
    while (idx < buff_size && buffer[idx] != '\0' && buffer[idx] != EOF) {
        if (new_word) {
            free_word(&new_word);
        }
        init_word(&new_word);
        new_word->size = 0;
        while (buffer[idx] != ' ' &&
               buffer[idx] != '\n' &&
               buffer[idx] != '\t' &&
               buffer[idx] != EOF &&
               buffer[idx] != '\0') {
            new_word->characters[new_word->size] = buffer[idx];
            new_word->size++;
            idx++;
        }
        idx++; // move past the space
        new_word->characters[new_word->size] = '\0';
        number_of_words++;
        printf("content: %s\nsize: %i\n\n", new_word->characters, new_word->size);
    }

    return number_of_words;
}

int break_into_paragraphs(char* buffer) {
    int number_of_paragraphs = 0;
    return number_of_paragraphs;
}
