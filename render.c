#include <string.h>
#include <curses.h>
#include "structs.h"
#include "helpers.h"
#include "buffers.h"

void print_statusline(
        int max_y, int max_x,
        int line_idx, int total_lines, int buffer_idx,
        int command_mode, int insert,
        struct Line* current_line) {
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
    printw(" | current buffer size: %ui", current_line->buffer->allocated);

    for (i = 0; i < max_x; i++) {
        addch(' ');
    }

    attroff(A_STANDOUT);
}

void render_formatted_lines(
        struct Line* head,
        int left_margin,
        int renderable_line_length) {
    Buffer* render_buffer = NULL;
    struct Line* line_to_render = head;
    /* print numbers for non-empty lines */
    /* we increment idx first so we don't start at 0 */
    int idx_of_line_to_render = 1;
    int cy = 0;

    while (line_to_render != NULL) {
        /* subtract 4 from left_margin due to paragraph denominator length */
        int cx = left_margin - 4;
        int current_line_idx = 0;
        char* content = line_to_render->buffer->content;

        /* print paragraph denomination */
        move(cy, cx);
        attron(A_ITALIC);
        attron(A_DIM);
        printw("%3i ", idx_of_line_to_render);
        attroff(A_ITALIC);
        attroff(A_DIM);

        /* increment paragraph denomination counter */
        idx_of_line_to_render++;

        /* print line content */
        cx = left_margin;
        move(cy, cx);
        while (current_line_idx < line_to_render->buffer->allocated &&
               '\0' != content[current_line_idx] &&
               '\n' != content[current_line_idx]) {
            int added_extra_char = 0;
            int render_idx = 0;

            /* zero our render buffer */
            init_buffer(&render_buffer);

            while (current_line_idx < line_to_render->buffer->allocated &&
                   ' ' != content[current_line_idx] &&
                   '-' != content[current_line_idx] &&
                   '\n' != content[current_line_idx] &&
                   '\0' != content[current_line_idx]) {
                process_character_for_buffer_with_nullchar(
                        render_buffer,
                        render_idx,
                        content[current_line_idx],
                        0);
                render_idx++;
                current_line_idx++;
                cx++;
                /* TODO: fix (it gets overwritten by next char atm */
                /* this code generated another '-' on the newline... why? */
                if (render_idx > 10 && cx - left_margin > renderable_line_length - 1) {
                    process_character_for_buffer_with_nullchar(
                            render_buffer,
                            render_idx,
                            '-',
                            0);
                    break;
                }
            }
            if (cx - left_margin > renderable_line_length) {
                cx = left_margin;
                cy++;
                move(cy, cx);
            }
            /* adds space, hypen, or other 'ignored' char from loop above */
            if (content[current_line_idx] != '\n' && content[current_line_idx] != '\0') {
                process_character_for_buffer_with_nullchar(
                        render_buffer,
                        render_idx,
                        content[current_line_idx],
                        0);
                added_extra_char++;
            }
            printw("%s", render_buffer->content);
            /* we need to getyx so we can store current cx after print */
            getyx(stdscr, cy, cx);
            current_line_idx++;
            /* if >1 we added the ignored char above so we still increment cx */
            if (added_extra_char) {
                cx++;
            }
        }
        line_to_render = line_to_render->next;
        cy += 2;
    }
}

void finalize_cursor_position(
        int* cy, int* cx, int left_margin,
        int buffer_idx, int line_idx, int renderable_line_length,
        struct Line* head, struct Line* current_line) {
    int current_line_idx = 0;
    int line_counter = 0;
    char* content = current_line->buffer->content;

    *cy = line_idx * 2;
    while (line_counter < line_idx) {
        Buffer* count_buffer = find_line_at_index(head, line_counter)->buffer;
        *cy += (strlen(count_buffer->content) - 1) / renderable_line_length;
        line_counter++;
    }

    *cx = left_margin;
    move(*cy, *cx);
    while(current_line_idx < buffer_idx) {
        int current_word_length = 0;
        while (current_line_idx < buffer_idx &&
               ' ' != content[current_line_idx] &&
               '-' != content[current_line_idx] &&
               '\n' != content[current_line_idx] &&
               '\0' != content[current_line_idx]) {
            current_word_length++;
            current_line_idx++;
            *cx += 1;
            if (current_word_length > 10 &&
                *cx - left_margin > renderable_line_length - 2) {
                break;
            }
        }
        if (*cx - left_margin > renderable_line_length - 1) {
            *cx = left_margin + current_word_length;
            *cy += 1;
            move(*cy, *cx);
        }
        if ((content[current_line_idx] == ' ' ||
            content[current_line_idx] == '-') &&
            buffer_idx != current_line_idx) {
            *cx += 1;
        }
        current_line_idx++;
    }
    move(*cy, *cx);
}
