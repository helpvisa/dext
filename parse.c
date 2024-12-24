#include <stdio.h>
#include <string.h>
#include "create_structures.h"
#include "structs.h"

int main(int argc, char* argv[]) {
    char line[1000000];
    memset(line, '\0', sizeof(line));
    int total_paragraphs = 0;
    int total_lines = 0;
    int total_words = 0;
    int total_chars = 0;
    int allocated_memory = 0;

    // create a body to hold our text
    struct Body* main_body = NULL;
    init_body(&main_body);

    while (fgets(line, sizeof(line), stdin)) {
        total_paragraphs = break_into_paragraphs(&main_body, line, sizeof(line));
    }

    // display formatted input + count totals
    allocated_memory += sizeof(struct Body);
    allocated_memory += sizeof(struct Paragraph*) * main_body->allocated;
    for (int p = 0; p < main_body->size; p++) {
        total_lines += main_body->paragraphs[p]->size;
        total_chars += main_body->paragraphs[p]->size - 1; // newlines
        allocated_memory += sizeof(struct Line*) * main_body->paragraphs[p]->allocated;
        for (int l = 0; l < main_body->paragraphs[p]->size; l++) {
            total_words += main_body->paragraphs[p]->formatted_lines[l]->size;
            total_chars += main_body->paragraphs[p]->formatted_lines[l]->size - 1; // spaces
            allocated_memory += sizeof(struct Word*) * main_body->paragraphs[p]->formatted_lines[l]->allocated;
            for (int w = 0; w < main_body->paragraphs[p]->formatted_lines[l]->size; w++) {
                total_chars += main_body->paragraphs[p]->formatted_lines[l]->words[w]->size;
                allocated_memory += sizeof(char) * main_body->paragraphs[p]->formatted_lines[l]->words[w]->allocated;
                printf("%s", main_body->paragraphs[p]->formatted_lines[l]->words[w]->characters);
            }
            /* printf("\n"); */
        }
        /* printf("\n\n"); */
    }
    printf("\n");
    printf("----------------------------------------------------\n");
    printf("|                DOCUMENT TOTALS                   |\n");
    printf("----------------------------------------------------\n");
    printf("%20i | paragraphs\n"
            "%20i | lines\n"
            "%20i | words\n"
            "%20i | characters\n",
            total_paragraphs, total_lines, total_words, total_chars);
    printf("----------------------------------------------------\n");
    printf("|  SOME ROUGH MEMORY ESTIMATIONS, ON THE LOW SIDE  |\n");
    printf("----------------------------------------------------\n");
    printf("%20i | bytes allocated\n", allocated_memory);
    printf("%20.5f | KB allocated\n", (double)allocated_memory / 1000);
    printf("%20.5f | MB allocated\n", (double)allocated_memory / 1000 / 1000);

    // free our lines
    free_body(&main_body);

    return 0;
}
