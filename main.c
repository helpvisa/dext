#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "structs.h"
#include "buffers.h"
#include "helpers.h"

int main(int argc, char* argv[]) {
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
    struct Line *first_line = malloc(sizeof(*first_line));
    first_line->buffer = NULL;
    first_line->next = NULL;
    if (init_buffer(&first_line->buffer) < 0) {
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
                int reverse_idx = buffer_idx;
                while (reverse_idx < buffer->allocated && buffer->content[reverse_idx] != '\0') {
                    buffer->content[reverse_idx] = buffer->content[reverse_idx + 1];
                    reverse_idx++;
                }
            } else if (line_idx >0) {
                line_idx--;
                struct Line* current_line = find_line_at_index(first_line, line_idx);
                buffer = current_line->buffer;
                buffer_idx = strlen(buffer->content);
            }
            break;
        case KEY_LEFT:
            if (buffer_idx > 0) {
                buffer_idx--;
            }
            break;
        case KEY_RIGHT:
            if (buffer->content[buffer_idx] != '\0') {
                buffer_idx++;
            }
            break;
        case KEY_UP:
            if (line_idx > 0) {
                line_idx--;
                struct Line* current_line = find_line_at_index(first_line, line_idx);
                buffer = current_line->buffer;
                int buffer_len = strlen(buffer->content);
                if (buffer_idx > buffer_len) {
                    buffer_idx = buffer_len;
                }
            }
            break;
        case KEY_DOWN:
            if (line_idx < total_lines) {
                line_idx++;
                struct Line* current_line = find_line_at_index(first_line, line_idx);
                buffer = current_line->buffer;
                int buffer_len = strlen(buffer->content);
                if (buffer_idx > buffer_len) {
                    buffer_idx = buffer_len;
                }
            }
            break;
        case '\n':
            // we have to do this first; we cannot declare right after label :/
            line_idx++;
            Buffer* new_buffer = NULL;
            if (init_buffer(&new_buffer) < 0) {
                exit(1);
            }
            buffer = new_buffer;
            struct Line* current_line = find_line_at_index(first_line, line_idx - 1);
            insert_line(&current_line, new_buffer);
            buffer_idx = 0;
            break;
        default:
            if (buffer_idx > 71) {
                line_idx++;
                buffer_idx = 0;
                if (line_idx > total_lines - 1) {
                    total_lines++;
                    Buffer* new_buffer = NULL;
                    if (init_buffer(&new_buffer) < 0) {
                        exit(1);
                    }
                    buffer = new_buffer;
                    struct Line* current_line = find_line_at_index(first_line, line_idx - 1);
                    insert_line(&current_line, new_buffer);
                } else {
                    struct Line* current_line = find_line_at_index(first_line, line_idx);
                    buffer = current_line->buffer;
                }
            }
            process_character_for_buffer(buffer, buffer_idx, c, insert);
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
        printw(" | current buffer size: %lu", buffer->allocated);
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
        struct Line* line_to_display = first_line;
        Buffer* buffer_to_display = NULL;
        while (NULL != line_to_display) {
            move(cy, cx);
            buffer_to_display = line_to_display->buffer;
            int i = 0;
            while (i < buffer_to_display->allocated &&
                   '\0' != buffer_to_display->content[i]) {
                if (buffer_to_display->content[i] == '\n') {
                    cy++;
                    cx = left_margin;
                    move(cy, cx);
                } else {
                    addch(buffer_to_display->content[i]);
                }
                i++;
            }
            line_to_display = line_to_display->next;
            cy++;
        }

        // finalize cursor position
        cy = line_idx;
        cx = left_margin + buffer_idx;
        move(cy, cx);
        refresh();
    }

    // clean up ncurses
    endwin();

    // debug print
    struct Line* current_line = first_line;
    int number_of_prints = 0;
    while (NULL != current_line) {
        for (int w = 0; w < strlen(current_line->buffer->content); w++) {
            if (current_line->buffer->content[w] == '\n') {
                current_line->buffer->content[w] = ' ';
            }
        }
        printf("%3i: %s\n", number_of_prints, current_line->buffer->content);
        current_line = current_line->next;
        number_of_prints++;
    }

    // free up allocations
    remove_line(&first_line, &first_line);

    return 0;
}
