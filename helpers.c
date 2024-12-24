#include <stdlib.h>
#include <string.h>

int expand_buffer(char** buffer, int buffsize, int step) {
    int new_buffsize = buffsize + step;
    char* new_allocation = realloc(*buffer, new_buffsize * sizeof(char));
    if (NULL == new_allocation) {
        return -1;
    }
    *buffer = new_allocation;
    memset(*buffer + buffsize, 0, step * sizeof(char));

    return new_buffsize;
}

int expand_line_buffer(char*** line_buffer, int linecount, int step) {
    int new_linecount = linecount + step;
    char** new_allocation = realloc(*line_buffer, new_linecount * sizeof(char*));
    if (NULL == new_allocation) {
        return -1;
    }
    *line_buffer = new_allocation;
    memset(*line_buffer + linecount, 0, step * sizeof(char*));

    return new_linecount;
}
