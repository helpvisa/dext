#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "structs.h"
#include "buffers.h"
#include "helpers.h"

static const int renderable_line_length = 72;

int main(int argc, char* argv[]) {
    // use to paint *immediately* upon first launch
    int is_first_loop = 1;
    // general editor modes
    int command_mode = 1;
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

        if (max_y < 8 || max_x < renderable_line_length) {
            curs_set(FALSE);
            erase();
            move(0,0);
            printw("Please resize to at least:\n8 rows\n%i columns", renderable_line_length);
            refresh();
            continue;
        }

        // query user inputs
        // comments refer to the case below their line
        c = getch();
        // esc
        if (command_mode) {
            switch (c) {
            case '\e':
                nodelay(stdscr, TRUE);
                if (getch() == -1) {
                    curs_set(FALSE);
                    clear();
                    refresh();
                    move(max_y / 2 - 1, max_x / 2 - 12);
                    printw("PRESS ESC AGAIN TO QUIT");
                    move(max_y / 2, max_x / 2 - 12);
                    printw("OR ANYTHING ELSE TO NOT");
                    timeout(-1);
                    char new_order = getch();
                    switch (new_order) {
                    case '\e':
                        run_loop = 0;
                        break;
                    default:
                        break;
                    }
                }
                nodelay(stdscr, FALSE);
                break;
            case 'h':
            case KEY_BACKSPACE:
            case KEY_LEFT:
                if (buffer_idx > 0) {
                    buffer_idx--;
                } else if (line_idx > 0) {
                    line_idx--;
                    current_line = find_line_at_index(first_line, line_idx);
                    buffer_idx = strlen(current_line->buffer->content);
                    if (buffer_idx > 0 && current_line->buffer->content[buffer_idx - 1] == '\n') {
                        buffer_idx--;
                    }
                }
                break;
            case 'l':
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
            case 'k':
            case KEY_UP:
                buffer_idx -= renderable_line_length;
                if (buffer_idx < 0) {
                    if (line_idx > 0) {
                        line_idx--;
                        current_line = find_line_at_index(first_line, line_idx);
                    }
                    buffer_idx = renderable_line_length + buffer_idx;
                    if (buffer_idx > strlen(current_line->buffer->content)) {
                        buffer_idx = strlen(current_line->buffer->content);
                        if (buffer_idx > 0 && current_line->buffer->content[buffer_idx - 1] == '\n') {
                            buffer_idx--;
                        }
                    }
                }
                break;
            case 'j':
            case KEY_DOWN:
                buffer_idx += renderable_line_length;
                if (buffer_idx > strlen(current_line->buffer->content)) {
                    if (line_idx < total_lines - 1) {
                        line_idx++;
                        current_line = find_line_at_index(first_line, line_idx);
                    }
                    buffer_idx %= renderable_line_length;
                    if (buffer_idx > strlen(current_line->buffer->content)) {
                        buffer_idx = strlen(current_line->buffer->content);
                        if (buffer_idx > 0 && current_line->buffer->content[buffer_idx - 1] == '\n') {
                            buffer_idx--;
                        }
                    }
                }
                break;
            case 'i':
                insert = 1;
                command_mode = 0;
                break;
            case 'a':
                insert = 1;
                command_mode = 0;
                if (buffer_idx < strlen(current_line->buffer->content)
                    && current_line->buffer->content[buffer_idx] != '\0') {
                        buffer_idx++;
                }
                break;
            case 'R':
                insert = 0;
                command_mode = 0;
                break;
            default:
                break;
            }
        } else {
            switch (c) {
            case '\e':
                /* nodelay(stdscr, TRUE); */
                command_mode = 1;
                /* nodelay(stdscr, FALSE); */
                break;
            // nothing
            case EOF:
                break;
            case KEY_BACKSPACE:
                if (buffer_idx > 0) {
                    buffer_idx--;
                    delete_character_from_buffer(current_line->buffer, buffer_idx);
                } else if (line_idx > 0) {
                    if (buffer_idx == 0 && current_line->buffer->content[buffer_idx] == '\n') {
                        delete_character_from_buffer(current_line->buffer, buffer_idx);
                    }
                    line_idx--;
                    current_line = find_line_at_index(first_line, line_idx);
                    buffer_idx = strlen(current_line->buffer->content);
                    if (buffer_idx > 0 && current_line->buffer->content[buffer_idx - 1] == '\n') {
                        buffer_idx--;
                    }
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
                    buffer_idx = strlen(current_line->buffer->content);
                    if (buffer_idx > 0 && current_line->buffer->content[buffer_idx - 1] == '\n') {
                        buffer_idx--;
                    }
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
            case KEY_UP:
                buffer_idx -= renderable_line_length;
                if (buffer_idx < 0) {
                    if (line_idx > 0) {
                        line_idx--;
                        current_line = find_line_at_index(first_line, line_idx);
                    }
                    buffer_idx = renderable_line_length + buffer_idx;
                    if (buffer_idx > strlen(current_line->buffer->content)) {
                        buffer_idx = strlen(current_line->buffer->content);
                        if (buffer_idx > 0 && current_line->buffer->content[buffer_idx - 1] == '\n') {
                            buffer_idx--;
                        }
                    }
                }
                break;
            case KEY_DOWN:
                buffer_idx += renderable_line_length;
                if (buffer_idx > strlen(current_line->buffer->content)) {
                    if (line_idx < total_lines - 1) {
                        line_idx++;
                        current_line = find_line_at_index(first_line, line_idx);
                    }
                    buffer_idx %= renderable_line_length;
                    if (buffer_idx > strlen(current_line->buffer->content)) {
                        buffer_idx = strlen(current_line->buffer->content);
                        if (buffer_idx > 0 && current_line->buffer->content[buffer_idx - 1] == '\n') {
                            buffer_idx--;
                        }
                    }
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
        }

        // rendering phase begins
        erase();

        // statusline
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
        printw(" | current buffer size: %lu", current_line->buffer->allocated);
        for (int i = 0; i < max_x; i++) {
            addch(' ');
        }
        attroff(A_STANDOUT);

        // print numbers for non-empty lines
        left_margin = max_x / 2 - 36;
        cy = 0;
        cx = left_margin - 6; // size of number idx
        for (int line_num = 0; line_num < total_lines; line_num++) {
            struct Line* this_line = find_line_at_index(first_line, line_num);
            move(cy, cx);
            attron(A_ITALIC);
            printw("%3i", line_num + 1);
            attroff(A_ITALIC);
            printw("   ");
            if (strlen(this_line->buffer->content) > renderable_line_length) {
                cy += strlen(this_line->buffer->content) / renderable_line_length;
            }
            cy += 2;
        }
        // prepare cursor
        curs_set(TRUE);
        cy = 0;
        cx = left_margin;
        // render lines and their buffers to the screen
        struct Line* line_to_render = first_line;
        while (line_to_render != NULL) {
            move(cy, cx);
            int i = 0;
            int wrap_counter = 1;
            while (i < line_to_render->buffer->allocated &&
                   '\0' != line_to_render->buffer->content[i] &&
                   '\n' != line_to_render->buffer->content[i]) {
                move(cy, cx);
                addch(line_to_render->buffer->content[i]);
                cx++;
                i++;
                wrap_counter++;
                if (wrap_counter > renderable_line_length) {
                    wrap_counter = 1;
                    cx = left_margin;
                    cy++;
                }
            }
            line_to_render = line_to_render->next;
            cx = left_margin;
            cy += 2;
        }

        // finalize cursor position
        cy = line_idx * 2;
        // this is brute force but it works...
        line_to_render = first_line;
        while (NULL != line_to_render) {
            cy += strlen(line_to_render->buffer->content) / renderable_line_length;
            line_to_render = line_to_render->next;
        }
        cx = left_margin + buffer_idx % renderable_line_length;
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
