#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "structs.h"
#include "create_structures.h"

#define BUFFER_ALLOC_INTERVAL 2048

int main(int argc, char* argv[]) {
    // general editor modes
    int insert = 1;

    // window and cursor tracking
    int max_x = 0, max_y = 0;
    int left_margin = 0, right_margin = 0;
    int cx = 0, cy = 0;
    int ox = 0, oy = 0;
    int c = '\0';

    // setup buffer
    int buffer_size = BUFFER_ALLOC_INTERVAL;
    char* buffer = NULL;
    buffer = malloc(buffer_size * sizeof(char));
    if (NULL == buffer) {
        return -1;
    }
    memset(buffer, 0, buffer_size);
    int buffer_idx = 0;

    // setup body structure
    int number_of_paragraphs = 0;
    int number_of_lines = 0;
    int number_of_words = 0;
    int number_of_chars = 0;
    struct Body* main_body = NULL;
    init_body(&main_body);

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
                while (buffer[reverse_idx] != '\0') {
                    buffer[reverse_idx] = buffer[reverse_idx + 1];
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
            if (buffer[buffer_idx] != '\0') {
                buffer_idx++;
                ox++;
            }
            break;
        default:
            if (buffer[buffer_idx] == '\0' || !insert) {
                buffer[buffer_idx] = c;
                if (ox < 0) {
                    ox++;
                }
            } else {
                int reverse_idx = buffer_idx;
                while (buffer[reverse_idx] != '\0') {
                    reverse_idx++;
                }
                while (reverse_idx >= buffer_idx) {
                    buffer[reverse_idx + 1] = buffer[reverse_idx];
                    reverse_idx--;
                }
                buffer[buffer_idx] = c;
            }
            buffer_idx++;
            break;
        }

        // process our buffer for rendering and info display
        free_body(&main_body);
        init_body(&main_body);
        number_of_paragraphs = break_into_paragraphs(&main_body, buffer, buffer_size);
        number_of_lines = 0;
        number_of_words = 0;
        number_of_chars = 0;

        // clear and render the contents of the buffer to the screen
        erase();

        // main body
        left_margin = max_x / 2 - 36;
        cy = 0;
        cx = left_margin;
        move(cy, cx);
        for (int p = 0; p < number_of_paragraphs; p++) {
            struct Paragraph* temp_paragraph = main_body->paragraphs[p];
            for (int l = 0; l < temp_paragraph->size; l++) {
                cx = left_margin;
                move(cy, cx);
                struct Line* temp_line = main_body->paragraphs[p]->formatted_lines[l];
                number_of_lines++;
                for (int w = 0; w < temp_line->size; w++) {
                    struct Word* temp_word = temp_line->words[w];
                    number_of_words++;
                    for (int c = 0; c < strlen(temp_word->characters); c++) {
                        number_of_chars++;
                        addch(temp_word->characters[c]);
                        if (temp_word->characters[c] == '\n') {
                            cy++;
                            cx = left_margin;
                        } else {
                            cx++;
                        }
                    }
                    // if we manually input newlines, this ensures proper
                    // cursor placement
                    move(cy, cx);
                }
                // move cursor down for all but last line
                if (l < temp_paragraph->size - 1) {
                    cy++;
                }
            }
        }

        // statusline
        move(max_y - 1, 0);
        printw("paragraphs: %i | lines: %i | words: %i | characters: %i | ",
               number_of_paragraphs, number_of_lines, number_of_words, number_of_chars);
        if (insert) {
            printw("INSERTING");
        } else {
            printw("REPLACING");
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
    free(buffer);
    buffer = NULL;
    free_body(&main_body);
    free(main_body);
    main_body = NULL;

    return 0;
}
