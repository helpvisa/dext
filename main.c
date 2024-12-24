#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "structs.h"
#include "buffers.h"
#include "create_structures.h"

#define BUFFER_ALLOC_INTERVAL 16

int main(int argc, char* argv[]) {
    // general editor modes
    int insert = 1;

    // window and cursor tracking
    int max_x = 0, max_y = 0;
    int left_margin = 0, right_margin = 0;
    int cx = 0, cy = 0;
    int ox = 0, oy = 0;
    int c = '\0';

    // set up lines and their buffers
    struct Buffer* buffer = NULL;
    if (init_buffer(&buffer, BUFFER_ALLOC_INTERVAL) < 0) {
        exit(1);
    }
    int line_idx = 0;
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

        // reset memory counter
        int total_allocated_bytes_of_memory = 0;

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

        // process our buffer for rendering and info display
        free_body(&main_body);
        init_body(&main_body);
        number_of_paragraphs = break_into_paragraphs(&main_body, buffer->content, buffer->allocated);
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
        total_allocated_bytes_of_memory += sizeof(*main_body);
        for (int p = 0; p < number_of_paragraphs; p++) {
            struct Paragraph* temp_paragraph = main_body->paragraphs[p];
            total_allocated_bytes_of_memory += sizeof(*temp_paragraph);
            for (int l = 0; l < temp_paragraph->size; l++) {
                cx = left_margin;
                move(cy, cx);
                struct Line* temp_line = main_body->paragraphs[p]->formatted_lines[l];
                number_of_lines++;
                total_allocated_bytes_of_memory += sizeof(*temp_line);
                for (int w = 0; w < temp_line->size; w++) {
                    struct Word* temp_word = temp_line->words[w];
                    number_of_words++;
                    total_allocated_bytes_of_memory += sizeof(*temp_word);
                    for (int c = 0; c < strlen(temp_word->characters); c++) {
                        number_of_chars++;
                        total_allocated_bytes_of_memory += sizeof(char);
                        addch(temp_word->characters[c]);
                        if (temp_word->characters[c] == '\n') {
                            cy++;
                            cx = left_margin;
                        } else {
                            cx++;
                        }
                    }
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
        printw(" | buffer_idx: %i", buffer_idx);

        // if buffer_idx equals the currently allocated value, it needs expansion
        if (buffer_idx >= buffer->allocated) {
            allocate_memory_to_buffer(&buffer, BUFFER_ALLOC_INTERVAL);
        }
        printw(" | buffer size: %i", buffer->allocated);
        total_allocated_bytes_of_memory += buffer->allocated * sizeof(char);
        printw(" | memory allocated: %i bytes (%.2f kilobytes)",
               total_allocated_bytes_of_memory,
               (double)total_allocated_bytes_of_memory / 1000);

        // finalize cursor position
        cy += oy;
        cx += ox;
        move(cy, cx);
        refresh();
    }

    // clean up ncurses
    endwin();

    // free up allocations
    free_buffer(&buffer);
    free_body(&main_body);
    free(main_body);
    main_body = NULL;

    return 0;
}
