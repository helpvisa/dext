#include <stdio.h>
#include <string.h>
#include "create_structures.h"
#include "structs.h"

int main(int argc, char* argv[]) {
    char line[1000000];
    memset(line, '\0', sizeof(line));
    int total_words = 0;
    int bytes_allocated = 0;

    // create a line struct to hold our words
    struct Line* test_line = NULL;
    init_line(&test_line);

    while (fgets(line, sizeof(line), stdin)) {
        total_words = break_into_words(&test_line, line, sizeof(line));
    }

    test_line->size = total_words;
    for (int i = 0; i < test_line->size; i++) {
        printf("word: %s\nsize: %i\n\n", test_line->words[i]->characters, test_line->words[i]->size);
        bytes_allocated += test_line->words[i]->allocated;
    }
    printf("\ntotal words: %i\n", total_words);

    bytes_allocated += test_line->allocated * sizeof(*test_line->words[0]);
    bytes_allocated += sizeof(*test_line);
    printf("total bytes of allocated memory: %i\n", bytes_allocated);

    // free our lines
    free_line(&test_line);

    return 0;
}
