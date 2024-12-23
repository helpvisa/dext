#include <stdio.h>
#include <string.h>
#include "create_structures.h"

int main(int argc, char* argv[]) {
    char line[100];
    memset(line, '\0', sizeof(line));
    int total_words = 0;
    while (fgets(line, sizeof(line), stdin)) {
        total_words = break_into_words(NULL, line, sizeof(line));
    }
    printf("total words: %i\n", total_words);

    return 0;
}
