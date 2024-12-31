#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "structs.h"
#include "buffers.h"
#include "helpers.h"
#include "render.h"

static const int renderable_line_length = 72;

int main(int argc, char* argv[]) {
    /* use to paint *immediately* upon first launch */
    int is_first_loop = 1;
    int run_loop = 1;
    /* general editor modes */
    int command_mode = 1;
    int insert = 1;

    /* window and cursor tracking */
    /* c2 is for 'two-order' commands */
    int max_x = 0, max_y = 0;
    int left_margin = 0;
    int cx = 0, cy = 0;
    int c = '\0';
    int c2 = '\0';

    /* set up lines and their buffers */
    int total_lines = 1;
    int line_idx = 0;
    int buffer_idx = 0;
    struct Line* first_line = malloc(sizeof(*first_line));
    struct Line* current_line = first_line;

    first_line->buffer = NULL;
    if (init_buffer(&first_line->buffer) < 0) {
        exit(1);
    }
    first_line->next = NULL;

    /* setup curses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(10);

    /* input loop */
    while(run_loop) {
        if (is_first_loop) {
            timeout(1);
            is_first_loop = 0;
        } else {
            timeout(1000);
        }
        getmaxyx(stdscr, max_y, max_x);

        if (max_y < 8 || max_x < renderable_line_length + 8) {
            curs_set(FALSE);
            erase();
            move(0,0);
            printw("Please resize to at least:\n8 rows\n%i columns", renderable_line_length + 8);
            refresh();
            continue;
        }

        /* query user inputs */
        /* comments refer to the case below their line */
        c = getch();
        /* esc */
        if (command_mode) {
            switch (c) {
            case 'q':
                nodelay(stdscr, TRUE);
                if (getch() == -1) {
                    curs_set(FALSE);
                    clear();
                    refresh();

                    move(max_y / 2 - 1, max_x / 2 - 12);
                    printw("PRESS 'Q' KEY AGAIN TO QUIT");
                    move(max_y / 2, max_x / 2 - 12);
                    printw("PRESS ANY OTHER KEY TO STAY");

                    timeout(-1);

                    c2 = getch();
                    switch (c2) {
                    case 'q':
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
            /* now that line-wrapping works, maybe we can reverse-engineer */
            /* the cx and cy position to determine appropriate */
            /* buffer and line indices */
            case 'k':
            case KEY_UP:
                move_cursor_up_formatted_line(
                        cx, cy, left_margin,
                        &buffer_idx, &line_idx, renderable_line_length,
                        first_line, &current_line);
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
            /* 27 is ASCII code for escape key */
            case 27:
                command_mode = 1;
                break;
            /* nothing */
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
                create_and_insert_line(&current_line);
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

        /* rendering phase begins */
        erase();

        /* statusline */
        print_statusline(
                max_y, max_x,
                line_idx, total_lines, buffer_idx,
                command_mode, insert,
                current_line);

        /* prepare cursor */
        left_margin = max_x / 2 - 36;
        /* curs_set(TRUE); */
        render_formatted_lines(
                first_line,
                left_margin,
                renderable_line_length);

        finalize_cursor_position(
                &cy, &cx, left_margin,
                buffer_idx, line_idx, renderable_line_length,
                first_line, current_line);
        refresh();
    }

    /* clean up curses */
    endwin();

    /* print the results of our buffers for testing + fun */
    current_line = first_line;
    while (NULL != current_line) {
        printf("%s\n", current_line->buffer->content);
        current_line = current_line->next;
    }

    /* free up allocations */
    current_line = first_line;
    while (NULL != current_line) {
        struct Line* line_to_remove = current_line;
        current_line = current_line->next;
        remove_line(&first_line, line_to_remove);
    }

    return 0;
}
