#include <stdlib.h>
#include <string.h>
#include "structs.h"

int init_buffer(struct Buffer** new_buffer, int allocation) {
    *new_buffer = malloc(sizeof(**new_buffer));
    if (NULL == *new_buffer) {
        return -1;
    }
    memset(*new_buffer, 0, sizeof(**new_buffer));

    (*new_buffer)->content = malloc(allocation * sizeof(char));
    if (NULL == (*new_buffer)->content) {
        return -1;
    }
    memset((*new_buffer)->content, 0, allocation * sizeof(char));

    (*new_buffer)->allocated = allocation;

    return 0;
}

int allocate_memory_to_buffer(struct Buffer** buffer, int step) {
    int old_alloc_count = (*buffer)->allocated;
    int new_alloc_count = old_alloc_count + step;
    char* new_allocation = NULL;
    new_allocation = realloc((*buffer)->content,
                             new_alloc_count * sizeof(char));
    if (NULL == new_allocation) {
        return -1;
    }

    (*buffer)->content = new_allocation;
    (*buffer)->allocated = new_alloc_count;
    memset((*buffer)->content + old_alloc_count, 0, step);

    return 0;
}

void free_buffer(struct Buffer** dead_buffer) {
    free((*dead_buffer)->content);
    (*dead_buffer)->content = NULL;
    free(*dead_buffer);
    *dead_buffer = NULL;
}
