#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "structs.h"
#include "buffers.h"
#include "helpers.h"

static const int renderable_line_length = 72;

int main(int argc, char* argv[]) {
    /* use to paint *immediately* upon first launch */
    int is_first_loop = 1;
    /* general editor modes */
    int command_mode = 1;
    int insert = 1;

    /* window and cursor tracking */
    int max_x = 0, max_y = 0;
    int left_margin = 0;
    int cx = 0, cy = 0;
    int c = '\0';

    /* set up lines and their buffers */
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
    /* allocate a buffer which will be used only for rendering */
    Buffer* render_buffer = NULL;
    init_buffer(&render_buffer);

    /* setup curses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(10);

    /* input loop */
    int run_loop = 1;
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
                    char new_order = getch();
                    switch (new_order) {
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

        /* rendering phase begins */
        erase();

        /* statusline */
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
        printw(" | current buffer size: %llu", current_line->buffer->allocated);
        int i;
        for (i = 0; i < max_x; i++) {
            addch(' ');
        }
        attroff(A_STANDOUT);

        /* prepare cursor */
        left_margin = max_x / 2 - 36;
        curs_set(TRUE);
        cy = 0;
        /* render lines and their buffers to the screen */
        struct Line* line_to_render = first_line;
        int idx_of_line_to_render = 0;
        while (line_to_render != NULL) {
            /* print numbers for non-empty lines */
            /* we increment idx first, so we don't start at 0 */
            idx_of_line_to_render++;
            cx = left_margin - 6; /* size of number idx */
            move(cy, cx);
            attron(A_ITALIC);
            attron(A_DIM);
            printw("%3i", idx_of_line_to_render);
            attroff(A_ITALIC);
            attroff(A_DIM);
            printw("   ");
            /* print actual content */
            cx = left_margin;
            move(cy, cx);
            char* content = line_to_render->buffer->content;
            int i = 0;
            while (i < line_to_render->buffer->allocated &&
                   '\0' != content[i] &&
                   '\n' != content[i]) {
                /* find the word to print */
                int render_idx = 0;
                /* clear out our render buffer */
                init_buffer(&render_buffer);
                while (i < line_to_render->buffer->allocated &&
                       ' ' != content[i] &&
                       '-' != content[i] &&
                       '\n' != content[i] &&
                       '\0' != content[i]) {
                    process_character_for_buffer_with_nullchar(
                        render_buffer,
                        render_idx,
                        content[i],
                        0
                    );
                    render_idx++;
                    i++;
                    cx++;
                    /* TODO: fix (it gets overwritten atm) */
                    /* this code sometimes generates another - on the newline... why? */
                    if (render_idx > 10 && cx - left_margin > renderable_line_length - 1) {
                        process_character_for_buffer_with_nullchar(
                            render_buffer,
                            render_idx,
                            '-',
                            0
                        );
                        break;
                    }
                }
                if (cx - left_margin > renderable_line_length) {
                    cx = left_margin;
                    cy++;
                    move(cy, cx);
                }
                /* this adds a space, hyphen, or other 'ignored' char */
                int added_extra_char = 0;
                if (content[i] != '\n' && content[i] != '\0') {
                    process_character_for_buffer_with_nullchar(
                        render_buffer,
                        render_idx,
                        content[i],
                        0
                    );
                    added_extra_char++;
                }
                printw("%s", render_buffer->content);
                /* we need to getyx so we can store current cx after print */
                getyx(stdscr, cy, cx);
                i++;
                /* we added the ignored char above, so we still increment cx */
                if (added_extra_char) {
                    cx++;
                }
            }
            line_to_render = line_to_render->next;
            cy += 2;
        }
        /* reset attributes */
        attroff(A_ITALIC);

        /* finalize cursor position */
        cy = line_idx * 2;
        int line_counter = 0;
        while (line_counter < line_idx) {
            Buffer* count_buffer = find_line_at_index(first_line, line_counter)->buffer;
            cy += (strlen(count_buffer->content) - 1) / renderable_line_length;
            line_counter++;
        }
        /* this is brute force but it works... */
        cx = left_margin;
        move(cy, cx);
        char* content = current_line->buffer->content;
        int idx = 0;
        while (idx < buffer_idx) {
            int curr_word_length = 0;
            while (idx < buffer_idx &&
                   ' ' != content[idx] &&
                   '-' != content[idx] &&
                   '\n' != content[idx] && 
                   '\0' != content[idx]) {
                curr_word_length++;
                idx++;
                cx++;
                if (curr_word_length > 10 && cx - left_margin > renderable_line_length - 2) {
                    break;
                }
            }
            if (cx - left_margin > renderable_line_length - 1) {
                cx = left_margin + curr_word_length;
                cy++;
                move(cy, cx);
            }
            if (content[idx] == ' ' || content[idx] == '-') {
                cx++;
            }
            idx++;
        }
        move(cy, cx);
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
