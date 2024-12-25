#ifndef BUFFERS_H_
#define BUFFERS_H_

#include <stdint.h>
#include "structs.h"

void free_buffer(Buffer **buffer);
uint64_t init_buffer(Buffer **buffer, uint64_t initial_allocation);
uint64_t expand_buffer(Buffer **buffer, uint64_t expansion_amount);

void add_line_to_beignning(struct Line **head, Buffer *buffer);
void add_line_to_end(struct Line **head, Buffer *buffer);
void remove_line(struct Line **head, struct Line **to_remove);

#endif
