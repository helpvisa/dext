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
    int left_margin = 0;
    int cx = 0, cy = 0;
    int c = '\0';

    // set up lines and their buffers
    int total_lines = 1;
    int line_idx = 0;
    int buffer_idx = 0;
    struct Line* first_line = malloc(sizeof(*first_line));
    first_line->buffer = NULL;
    if (init_buffer(&first_line->buffer) < 0) {
        exit(1);
    }
    first_line->next = NULL;
    struct Line* current_line = first_line;

    // setup ncurses
    initscr();
    cbreak();
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

        if (max_y < 8 || max_x < 72) {
            curs_set(FALSE);
            erase();
            move(0,0);
            printw("Please resize to at least:\n8 rows\n72 columns");
            refresh();
            continue;
        }

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
                }
            }
            break;
        case KEY_LEFT:
            if (buffer_idx > 0) {
                buffer_idx--;
            }
            break;
        case KEY_RIGHT:
            if (current_line->buffer->content[buffer_idx] != '\0') {
                buffer_idx++;
            }
            break;
        case KEY_UP:
            break;
        case KEY_DOWN:
            break;
        case '\n':
            // we have to do this first; we cannot declare right after label :/
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
        /* printw(" | total lines: %i", total_lines); */
        printw(" | buffer_idx: %i", buffer_idx);
        printw(" | current buffer size: %lu", current_line->buffer->allocated);
        for (int i = 0; i < max_x; i++) {
            addch(' ');
        }
        attroff(A_STANDOUT);

        // prepare cursor
        curs_set(TRUE);
        left_margin = max_x / 2 - 36;
        cy = 0;
        cx = left_margin;
        // render lines and their buffers to the screen
        move(cy, cx);
        int i = 0;
        while (i < current_line->buffer->allocated &&
               '\0' != current_line->buffer->content[i]) {
            if (current_line->buffer->content[i] == '\n') {
                cy++;
                cx = left_margin;
                move(cy, cx);
            } else {
                addch(current_line->buffer->content[i]);
            }
            i++;
        }

        // finalize cursor position
        cy = line_idx;
        cx = left_margin + buffer_idx - (line_idx * 72);
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
        remove_line(&first_line, &line_to_remove);
    }

    return 0;
}
