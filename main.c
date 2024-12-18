#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    // setup ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // window and cursor tracking
    int max_x = 0, max_y = 0;
    int cx = 0, cy = 0;
    int c = '\0';

    // create a line that holds characters
    int number_of_lines = 32;
    char** lines = malloc(number_of_lines);
    if (!lines) {
        printf("Could not allocate memory!");
        exit(1);
    }
    for (int i = 0; i < number_of_lines; i++) {
        lines[i] = malloc(32);
        if (!lines[i]) {
            printf("Could not allocate memory!");
            // TODO: register cleanup functions with atexit
            exit(1);
        }
    }
    int line_idx = 0;
    int curr_line_idx = 0;

    // set timeout
    while(1) {
        timeout(33);
        getyx(stdscr, cy, cx);
        getmaxyx(stdscr, max_y, max_x);

        // query user inputs
        // comments refer to the case below their line
        c = getch();
        switch (c) {
        case EOF:
        // esc
        case 27:
            break;
        case KEY_BACKSPACE:
            move(cy, cx - 1);
            delch();
            lines[line_idx][curr_line_idx] = '\0';
            curr_line_idx--;
            break;
        case KEY_ENTER:
            move(cy + 1, cx);
            lines[line_idx][curr_line_idx] = '\0';
            // reset the cursor to start (TODO: be smarter about this)
            curr_line_idx = 0;
            line_idx++;
        default:
            lines[line_idx][curr_line_idx] = c;
            addch(lines[line_idx][curr_line_idx]);
            curr_line_idx++;
            break;
        }

        refresh();
    }

    // free allocations
    for (int i = 0; i < number_of_lines; i++) {
        free(lines[i]);
    }
    free(lines);

    endwin();
    return 0;
}
