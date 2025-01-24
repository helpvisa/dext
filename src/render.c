#include <string.h>
#include <curses.h>
#include "structs.h"
/* #include "helpers.h" */
/* #include "buffers.h" */

void print_statusline(
        int max_y, int max_x,
        int line_idx, int total_lines, int buffer_idx, int preferred_idx,
        int command_mode, int insert,
        struct Line* current_line,
        char* filepath) {
    /* use to fill blank screen space */
    int i;

    attron(A_STANDOUT);
    move(max_y - 1, 0);

    if (command_mode) {
        printw("NORMAL");
    } else if (insert) {
        printw("INSERTING");
    } else {
        printw("REPLACING");
    }

    printw(" | line_idx: %i", line_idx);
    printw(" | total lines: %i", total_lines);
    printw(" | buffer_idx: %i", buffer_idx);
    printw(" | preferred_idx: %i", preferred_idx);
    printw(" | current buffer size: %u", current_line->buffer->allocated);
    printw(" | editing '%s'", filepath);

    for (i = 0; i < max_x; i++) {
        addch(' ');
    }

    attroff(A_STANDOUT);
}

void render_lines(
        struct Line* head,
        int left_margin,
        int renderable_line_length) {
    struct Line* line_to_render = head;
    /* print numbers for non-empty lines */
    /* we increment idx first so we don't start at 0 */
    int idx_of_line_to_render = 1;
    int cy = 0;

    while (line_to_render != NULL) {
        /* subtract 4 from left_margin due to line denominator length */
        int cx = left_margin - 4;
        char* content = line_to_render->buffer->content;

        /* print line denomination */
        move(cy, cx);
        attron(A_DIM);
        printw("%3i ", idx_of_line_to_render);
        attroff(A_DIM);

        /* increment line denomination counter */
        idx_of_line_to_render++;

        /* print line content */
        cx = left_margin;
        move(cy, cx);
        printw("%s", content);
        line_to_render = line_to_render->next;
        cy++;
    }
}

void finalize_cursor_position(
        int* cy, int* cx, int left_margin,
        int buffer_idx, int line_idx, int renderable_line_length,
        struct Line* head, struct Line* current_line) {
    move(line_idx, buffer_idx + left_margin);
}
