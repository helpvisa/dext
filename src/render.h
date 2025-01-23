#ifndef RENDER_H_
#define RENDER_H_

#include "structs.h"

void print_statusline(
        int max_y, int max_x,
        int line_idx, int total_lines, int buffer_idx, int preferred_idx,
        int command_mode, int insert,
        struct Line* current_line,
        char* filepath);

void render_formatted_lines(
        struct Line* head,
        int left_margin,
        int renderable_line_length);

void finalize_cursor_position(
        int* cy, int* cx, int left_margin,
        int buffer_idx, int line_idx, int renderable_line_length,
        struct Line* head, struct Line* current_line);

#endif
