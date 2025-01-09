#ifndef IO_H_
#define IO_H_

#include <stdio.h>
#include "structs.h"

int load_file(char* filepath, struct Line** head, int* total_lines);
int save_file(char* filepath, struct Line* head);

#endif
