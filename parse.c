#include <stdio.h>
#include <string.h>
#include "create_structures.h"
#include "structs.h"

int main(int argc, char* argv[]) {
    char line[1000000];
    memset(line, '\0', sizeof(line));
    int total_paragraphs = 0;
    int total_words = 0;

    // create a body to hold our text
    struct Body* test_body = NULL;
    init_body(&test_body);

    while (fgets(line, sizeof(line), stdin)) {
        total_paragraphs = break_into_paragraphs(&test_body, line, sizeof(line));
    }

    for (int i = 0; i < test_body->size; i++) {
        for (int w = 0; w < test_body->paragraphs[i]->content->size; w++) {
            struct Line* temp_line = test_body->paragraphs[i]->content;
            total_words++;
        }
    }
    printf("%10i paragraphs\n%10i words\n", total_paragraphs, total_words);

    // free our lines
    free_body(&test_body);

    return 0;
}
