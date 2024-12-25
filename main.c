#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "structs.h"
#include "buffers.h"

#define ALLOC_STEP 8

int main(int argc, char* argv[]) {
    // general editor modes
    int insert = 1;

    // window and cursor tracking
    int max_x = 0, max_y = 0;
    int left_margin = 0;
    int cx = 0, cy = 0;
    int ox = 0, oy = 0;
    int c = '\0';

    // set up lines and their buffers
    int line_idx = 0;
    int buffer_idx = 0;
    struct Line *first_line = malloc(sizeof(*first_line));
    first_line->buffer = NULL;
    first_line->next = NULL;
    if (init_buffer(&first_line->buffer, ALLOC_STEP) < 0) {
        exit(1);
    }
    Buffer *buffer = first_line->buffer;

    // setup ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(10);

    // input loop
    int run_loop = 1;
    while(run_loop) {
        timeout(1000);
        getmaxyx(stdscr, max_y, max_x);

        // query user inputs
        // comments refer to the case below their line
        c = getch();
        switch (c) {
        // esc
        case '\e':
            nodelay(stdscr, TRUE);
            if (getch() == -1) {
                clear();
                refresh();
                move(max_y / 2 - 1, max_x / 2 - 7);
                curs_set(FALSE);
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
            curs_set(TRUE);
            nodelay(stdscr, FALSE);
            break;
        // nothing
        case EOF:
        case KEY_UP:
        case KEY_DOWN:
            break;
        case KEY_BACKSPACE:
            if (buffer_idx > 0) {
                buffer_idx--;
                int reverse_idx = buffer_idx;
                while (reverse_idx < buffer->allocated && buffer->content[reverse_idx] != '\0') {
                    buffer->content[reverse_idx] = buffer->content[reverse_idx + 1];
                    reverse_idx++;
                }
            }
            break;
        case KEY_LEFT:
            if (buffer_idx > 0) {
                buffer_idx--;
                ox--;
            }
            break;
        case KEY_RIGHT:
            if (buffer->content[buffer_idx] != '\0') {
                buffer_idx++;
                ox++;
            }
            break;
        default:
            if (buffer->content[buffer_idx] == '\0' || !insert) {
                buffer->content[buffer_idx] = c;
                if (ox < 0) {
                    ox++;
                }
            } else {
                int reverse_idx = buffer_idx;
                while (reverse_idx < buffer->allocated && buffer->content[reverse_idx] != '\0') {
                    reverse_idx++;
                }
                while (reverse_idx >= buffer_idx) {
                    buffer->content[reverse_idx + 1] = buffer->content[reverse_idx];
                    reverse_idx--;
                }
                buffer->content[buffer_idx] = c;
            }
            buffer_idx++;
            break;
        }

        // clear and render the contents of the buffer to the screen
        erase();

        // statusline
        move(max_y - 1, 0);
        if (insert) {
            printw("INSERTING");
        } else {
            printw("REPLACING");
        }
        printw(" | line_idx: %i", line_idx);
        printw(" | buffer_idx: %i", buffer_idx);

        // if buffer_idx equals the currently allocated value, it needs expansion
        if (buffer_idx >= buffer->allocated) {
            expand_buffer(&buffer, ALLOC_STEP);
        }
        printw(" | current buffer size: %i", buffer->allocated);

        // main body
        left_margin = max_x / 2 - 36;
        cy = 0;
        cx = left_margin;
        move(cy, cx);
        int i = 0;
        while (i < buffer->allocated && '\0' != buffer->content[i]) {
            addch(buffer->content[i]);
            cx++;
            i++;
        }

        // finalize cursor position
        cy += oy;
        cx += ox;
        move(cy, cx);
        refresh();
    }

    // clean up ncurses
    endwin();

    // free up allocations
    remove_line(&first_line, &first_line);

    return 0;
}
