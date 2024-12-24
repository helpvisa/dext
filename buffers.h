#ifndef BUFFERS_H_
#define BUFFERS_H_

#include "structs.h"

int init_buffer(struct Buffer** new_buffer, int allocation);
int allocate_memory_to_buffer(struct Buffer** buffer, int step);
void free_buffer(struct Buffer** dead_buffer);

#endif
