#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "structs.h"
#include "buffers.h"
#include "helpers.h"
#include "render.h"
#include "stringop.h"
#include "io.h"

static const int renderable_line_length = 72;

int main(int argc, char* argv[]) {
    /* generic reusable iterators */
    int i;

    /* track our currently edited file */
    char* filepath = malloc(4096);
    char* command_buffer = malloc(4096);

    /* use to paint *immediately* upon first launch */
    int is_first_loop = 1;
    int run_loop = 1;
    /* general editor modes */
    int command_mode = 1;
    int insert = 0;

    /* window and cursor tracking */
    /* c2 is for 'two-order' commands */
    int max_x = 0, max_y = 0;
    int left_margin = 0;
    int cx = 0, cy = 0;
    /* temp characters used to read user input */
    int c = '\0';
    int c2 = '\0';
    int c3 = '\0';

    /* set up lines and their buffers */
    int total_lines = 1;
    int line_idx = 0;
    int buffer_idx = 0;
    struct Line* first_line = malloc(sizeof(*first_line));
    struct Line* current_line = first_line;

    first_line->buffer = NULL;
    if (init_buffer(&first_line->buffer) < 0) {
        return 1;
    }
    first_line->next = NULL;

    /* make sure our initialized buffers are not NULL */
    if (NULL == filepath || NULL == command_buffer) {
        printf("ERR: Could not initialize command buffers. Exiting.");
        return 1;
    }

    /* was an argument to load a file provided? */
    if (argc > 1) {
        copy_string(filepath, argv[1], 4095);
        load_file(filepath, &first_line, &total_lines);
    } else {
        copy_string(filepath, "untitled.txt", 12);
    }

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
            timeout(2000);
        }
        getmaxyx(stdscr, max_y, max_x);

        if (max_y < 8 || max_x < renderable_line_length + 8) {
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
            /* quit the program (confirm and no-confirm) */
            case 'q':
                clear();
                refresh();

                move(max_y / 2 - 1, max_x / 2 - 12);
                printw("PRESS 'Q' KEY AGAIN TO QUIT");
                move(max_y / 2, max_x / 2 - 12);
                printw("PRESS ANY OTHER KEY TO STAY");

                /* timeout(-1); */

                c2 = getch();
                switch (c2) {
                case 'q':
                    run_loop = 0;
                    break;
                default:
                    break;
                }
                break;
            /* this should be changed to 'delete forward' */
            case 'x':
                run_loop = 0;
                break;
            /* extended commands with escape */
            /* 27 is ASCII code for escape key */
            case 27:
            case ':':
                clear();
                refresh();

                move(max_y / 2 - 1, max_x / 2 - 14);
                printw("INPUT A COMMAND OR PRESS ESC");

                /* disable timeout when entering extended commands */
                timeout(-1);

                c2 = getch();
                switch (c2) {
                /* change the filepath */
                case 27:
                    break;
                case 'r':
                    clear();
                    refresh();
                    move(max_y / 2 - 1, max_x / 2 - 7);
                    printw("NEW FILENAME:");
                    move(max_y / 2, max_x / 2 - 7);
                    /* we must track cursor bc of text entry here */
                    getyx(stdscr, cy, cx);
                    /* reset iterator */
                    i = 0;
                    c3 = getch();
                    while (c3 != '\n' && c3 != 27 && i < 4095) {
                        if (c3 != 127 && c3 != 8 && c3 != KEY_BACKSPACE) {
                            addch(c3);
                            cx++;
                            command_buffer[i] = c3;
                            i++;
                        } else if (i > 0) {
                            i--;
                            cx--;
                            move(cy, cx);
                            delch();
                            command_buffer[i] = '\0';
                        }
                        c3 = getch();
                    }
                    command_buffer[i] = '\0';
                    /* only accept rename if ESC was not pressed */
                    if (c3 != 27) {
                        copy_string(filepath, command_buffer, 4095);
                    }
                    break;
                case 'w':
                    /* set timeout so screen disappears organically */
                    timeout(1000);
                    clear();
                    refresh();
                    move(max_y / 2 - 1, max_x / 2 - 14);
                    printw("SAVED %i BYTES.", save_file(filepath, first_line));
                    /* briefly block input */
                    getch();
                    break;
                case 'o':
                    /* set timeout so screen disappears organically */
                    timeout(1000);
                    clear();
                    refresh();
                    move(max_y / 2 - 1, max_x / 2 - 14);
                    /* free loaded lines */
                    current_line = first_line->next;
                    while (NULL != current_line) {
                        struct Line* line_to_remove = current_line;
                        current_line = current_line->next;
                        remove_line(&first_line, line_to_remove);
                    }
                    /* re-init first_line and reset current_line */
                    free_buffer(&first_line->buffer);
                    first_line->buffer = NULL;
                    init_buffer(&first_line->buffer);
                    current_line = first_line;
                    /* finally we load */
                    printw("LOADED %i BYTES.", load_file(filepath, &first_line, &total_lines));
                    /* briefly block input */
                    getch();
                    break;
                default:
                    break;
                }

                /* reset timeout for main loop */
                timeout(2000);
                break;
            /* 127 and 8 are alternate keycodes for backspace */
            /* they allow backspace to work on NetBSD and FreeBSD */
            case 127:
            case 8:
            case 'h':
            case KEY_BACKSPACE:
            case KEY_LEFT:
                if (buffer_idx > 0) {
                    buffer_idx--;
                } else if (line_idx > 0) {
                    // set ridiculously high buffer_idx so it auto-wraps to end
                    buffer_idx = 999;
                    move_cursor_up_formatted_line(
                            &buffer_idx, &line_idx, insert,
                            first_line, &current_line, total_lines);
                }
                break;
            case 'l':
            case KEY_RIGHT:
                if (insert) {
                    if (current_line->buffer->content[buffer_idx] != '\0' &&
                        current_line->buffer->content[buffer_idx] != '\n') {
                        buffer_idx++;
                    } else if (line_idx < total_lines - 1) {
                        line_idx++;
                        current_line = find_line_at_index(first_line, line_idx);
                        buffer_idx = 0;
                    }
                } else {
                    if (current_line->buffer->content[buffer_idx + 1] != '\0' &&
                        current_line->buffer->content[buffer_idx + 1] != '\n') {
                        buffer_idx++;
                    } else if (line_idx < total_lines - 1) {
                        line_idx++;
                        current_line = find_line_at_index(first_line, line_idx);
                        buffer_idx = 0;
                    }
                }
                break;
            /* now that line-wrapping works, maybe we can reverse-engineer */
            /* the cx and cy position to determine appropriate */
            /* buffer and line indices */
            case 'k':
            case KEY_UP:
                move_cursor_up_formatted_line(
                        &buffer_idx, &line_idx, insert,
                        first_line, &current_line, total_lines);
                break;
            case 'j':
            case KEY_DOWN:
                move_cursor_down_formatted_line(
                        &buffer_idx, &line_idx, insert,
                        first_line, &current_line, total_lines);
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
                } else {
                    process_character_for_buffer(current_line->buffer, buffer_idx, ' ', insert);
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
                insert = 0;
                if (buffer_idx > 0 &&
                    (current_line->buffer->content[buffer_idx] == '\n' ||
                     current_line->buffer->content[buffer_idx] == '\0')) {
                    buffer_idx -= 1;
                }
                command_mode = 1;
                break;
            /* nothing */
            case EOF:
                break;
            /* 127 and 8 are alternate keycodes for backspace */
            /* they allow backspace to work on NetBSD and FreeBSD */
            case 127:
            case 8:
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
                    // set ridiculously high buffer_idx so it auto-wraps to end
                    buffer_idx = 999;
                    move_cursor_up_formatted_line(
                            &buffer_idx, &line_idx, insert,
                            first_line, &current_line, total_lines);
                }
                break;
            case KEY_RIGHT:
                if (insert) {
                    if (current_line->buffer->content[buffer_idx] != '\0' &&
                        current_line->buffer->content[buffer_idx] != '\n') {
                        buffer_idx++;
                    } else if (line_idx < total_lines - 1) {
                        line_idx++;
                        current_line = find_line_at_index(first_line, line_idx);
                        buffer_idx = 0;
                    }
                } else {
                    if (current_line->buffer->content[buffer_idx + 1] != '\0' &&
                        current_line->buffer->content[buffer_idx + 1] != '\n') {
                        buffer_idx++;
                    } else if (line_idx < total_lines - 1) {
                        line_idx++;
                        current_line = find_line_at_index(first_line, line_idx);
                        buffer_idx = 0;
                    }
                }
                break;
            case KEY_UP:
                move_cursor_up_formatted_line(
                        &buffer_idx, &line_idx, insert,
                        first_line, &current_line, total_lines);
                break;
            case KEY_DOWN:
                move_cursor_down_formatted_line(
                        &buffer_idx, &line_idx, insert,
                        first_line, &current_line, total_lines);
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
                if (buffer_idx > renderable_line_length - 1) {
                    line_idx++;
                    total_lines++;
                    create_and_insert_line(&current_line);
                    push_to_next_buffer(
                        current_line->next->buffer,
                        current_line->buffer,
                        buffer_idx
                    );
                    current_line = find_line_at_index(first_line, line_idx);
                    buffer_idx = 0;
                }
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
                current_line,
                filepath);

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

    /* free up allocations */
    free(filepath);
    current_line = first_line;
    while (NULL != current_line) {
        struct Line* line_to_remove = current_line;
        current_line = current_line->next;
        remove_line(&first_line, line_to_remove);
    }

    return 0;
}
