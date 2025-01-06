#ifndef BUFFERS_H_
#define BUFFERS_H_

#include "structs.h"

extern const int alloc_step;

void free_buffer(Buffer **buffer);
unsigned int init_buffer(Buffer **buffer);
unsigned int expand_buffer(Buffer **buffer);

void add_line_to_beignning(struct Line **head, Buffer *buffer);
void add_line_to_end(struct Line **head, Buffer *buffer);
void insert_line(struct Line **ref, Buffer *buffer);
void create_and_insert_line(struct Line **ref);
void remove_line(struct Line **head, struct Line *to_remove);

#endif
