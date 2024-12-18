#include <ncurses.h>

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

    // set timeout
    while(1) {
        timeout(33);
        getyx(stdscr, cy, cx);
        getmaxyx(stdscr, max_y, max_x);

        // query user inputs
        c = getch();
        switch (c) {
        case EOF:
            break;
        case KEY_BACKSPACE:
            move(cy, cx - 1);
            delch();
            break;
        default:
            addch(c);
            break;
        }

        refresh();
    }

    endwin();
    return 0;
}
