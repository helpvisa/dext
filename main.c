#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "structs.h"
#include "buffers.h"
#include "helpers.h"

int main(int argc, char* argv[]) {
    // use to paint *immediately* upon first launch
    int is_first_loop = 1;
    // general editor modes
    int insert = 1;

    // window and cursor tracking
    int max_x = 0, max_y = 0;
    int cx = 0, cy = 0;
    int c = '\0';

    // set up lines and their buffers
    int total_lines = 1;
    int line_idx = 0;
    int buffer_idx = 0;
    int view_offset_x = 0;
    int view_offset_y = 0;
    struct Line* first_line = malloc(sizeof(*first_line));
    first_line->buffer = NULL;
    if (init_buffer(&first_line->buffer) < 0) {
        exit(1);
    }
    first_line->next = NULL;
    struct Line* current_line = first_line;

    // setup ncurses
    initscr();
    if (has_colors()) {
        start_color();
    }
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    raw();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(10);

    // input loop
    int run_loop = 1;
    while(run_loop) {
        if (is_first_loop) {
            timeout(1);
            is_first_loop = 0;
        } else {
            timeout(1000);
        }
        getmaxyx(stdscr, max_y, max_x);

        // query user inputs
        // comments refer to the case below their line
        c = getch();
        switch (c) {
        // esc
        case '\e':
            nodelay(stdscr, TRUE);
            if (getch() == -1) {
                curs_set(FALSE);
                clear();
                refresh();
                move(max_y / 2 - 1, max_x / 2 - 7);
                printw("ISSUE AN ORDER");
                timeout(-1);
                char new_order = getch();
                switch (new_order) {
                case 'i':
                    insert = !insert;
                    break;
                case '\e':
                    run_loop = 0;
                    break;
                }
            }
            nodelay(stdscr, FALSE);
            break;
        // nothing
        case EOF:
            break;
        case KEY_BACKSPACE:
            if (buffer_idx > 0) {
                buffer_idx--;
                delete_character_from_buffer(current_line->buffer, buffer_idx);
            } else if (line_idx > 0) {
                line_idx--;
                current_line = find_line_at_index(first_line, line_idx);
                buffer_idx = strlen(current_line->buffer->content) - 1;
                if (line_idx < total_lines - 1) {
                    pilfer_character_from_buffer(current_line->next->buffer, current_line->buffer);
                    if (strlen(current_line->next->buffer->content) < 1) {
                        remove_line(&first_line, current_line->next);
                        total_lines--;
                    }
                }
            }
            break;
        case KEY_LEFT:
            if (buffer_idx > 0) {
                buffer_idx--;
            } else if (line_idx > 0) {
                line_idx--;
                current_line = find_line_at_index(first_line, line_idx);
                buffer_idx = strlen(current_line->buffer->content) - 1;
            }
            break;
        case KEY_RIGHT:
            if (current_line->buffer->content[buffer_idx] != '\0' &&
                current_line->buffer->content[buffer_idx] != '\n') {
                buffer_idx++;
            } else if (line_idx < total_lines - 1) {
                line_idx++;
                current_line = find_line_at_index(first_line, line_idx);
                buffer_idx = 0;
            }
            break;
        // why is this so hard.
        case KEY_UP:
            if (line_idx > 0) {
                line_idx--;
                current_line = find_line_at_index(first_line, line_idx);
                if (buffer_idx > strlen(current_line->buffer->content)) {
                    // we subtract 1 to account for the newline char
                    buffer_idx = strlen(current_line->buffer->content) - 1;
                }
            } else {
                buffer_idx = 0;
            }
            break;
        case KEY_DOWN:
            if (line_idx < total_lines - 1) {
                line_idx++;
                current_line = find_line_at_index(first_line, line_idx);
                if (buffer_idx > strlen(current_line->buffer->content)) {
                    buffer_idx = strlen(current_line->buffer->content);
                }
            } else {
                buffer_idx = strlen(current_line->buffer->content);
            }
            break;
        case '\n':
            process_character_for_buffer(current_line->buffer, buffer_idx, c, insert);
            line_idx++;
            buffer_idx++;
            total_lines++;
            Buffer* new_buffer = NULL;
            if (init_buffer(&new_buffer) < 0) {
                exit(1);
            }
            insert_line(&current_line, new_buffer);
            push_to_next_buffer(current_line->next->buffer, current_line->buffer, buffer_idx);
            current_line = find_line_at_index(first_line, line_idx);
            buffer_idx = 0;
            break;
        default:
            process_character_for_buffer(current_line->buffer, buffer_idx, c, insert);
            buffer_idx++;
            break;
        }

        // rendering phase begins
        erase();

        // statusline
        attron(A_STANDOUT);
        move(max_y - 1, 0);
        if (insert) {
            printw("INSERTING");
        } else {
            printw("REPLACING");
        }
        printw(" | line_idx: %i", line_idx);
        printw(" | total lines: %i", total_lines);
        printw(" | buffer_idx: %i", buffer_idx);
        printw(" | current buffer size: %lu", current_line->buffer->allocated);
        for (int i = 0; i < max_x; i++) {
            addch(' ');
        }
        attroff(A_STANDOUT);

        // prepare cursor
        cy = 0;
        // render lines and their buffers to the screen
        if (buffer_idx - view_offset_x > max_x - 7) {
            view_offset_x += max_x / 2;
        } else if (buffer_idx < max_x - 7) {
            view_offset_x = 0;
        } else if (buffer_idx - view_offset_x < 0) {
            view_offset_x -= max_x / 2;
        }
        while (cy < max_y - 1) {
            cx = 0;
            move(cy, cx);

            if (cy < total_lines) {
                attron(COLOR_PAIR(2));
                printw("%3i | ", cy + 1);
                attron(COLOR_PAIR(1));

                struct Line* line_to_render = find_line_at_index(first_line, cy);

                int i = view_offset_x;
                while (i < line_to_render->buffer->allocated &&
                        cx < max_x - 6 &&
                       '\0' != line_to_render->buffer->content[i] &&
                       '\n' != line_to_render->buffer->content[i]) {
                    addch(line_to_render->buffer->content[i]);
                    cx++;
                    i++;
                }
            } else {
                attron(COLOR_PAIR(3));
                printw("~");
                attron(COLOR_PAIR(1));
            }
            cy++;
        }

        // finalize cursor position
        curs_set(TRUE);
        cy = line_idx;
        cx = buffer_idx + 6 - view_offset_x;
        move(cy, cx);
        refresh();
    }

    // clean up ncurses
    endwin();

    // free up allocations
    current_line = first_line;
    while (NULL != current_line) {
        struct Line* line_to_remove = current_line;
        current_line = current_line->next;
        remove_line(&first_line, line_to_remove);
    }

    return 0;
}
