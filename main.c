#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

int main(int argc, char* argv[]) {
    // window and cursor tracking
    int max_x = 0, max_y = 0;
    int cx = 0, cy = 0;
    int c = '\0';

    // TODO: rework with structs (paragraph struct -> sentence struct -> word struct)
    // we store things in a buffer, then write to the screen and keep a data structure in tact
    // the buffer is small (32) and is analyzed+zeroed upon 'overflow' then stored in the data structure
    char buffer[32];
    memset(buffer, '\0', sizeof(buffer));
    int buffer_idx = 0;

    int number_of_sentences = 32;
    int sentence_malloc_interval = 64;

    char** sentences = malloc(number_of_sentences * sizeof(char*));
    if (!sentences) {
        printf("Could not allocate memory!");
        exit(1);
    }
    memset(sentences, 0, number_of_sentences * sizeof(char*));

    for (int i = 0; i < number_of_sentences; i++) {
        sentences[i] = malloc(sentence_malloc_interval * sizeof(char));
        if (!sentences[i]) {
            printf("Could not allocate memory!");
            // TODO: register cleanup functions with atexit
            exit(1);
        }

        // zero out line
        memset(sentences[i], 0, sentence_malloc_interval * sizeof(char));
    }

    // setup ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // input loop
    int run_loop = 1;
    while(run_loop) {
        timeout(33);
        getmaxyx(stdscr, max_y, max_x);

        // query user inputs
        // comments refer to the case below their line
        c = getch();
        switch (c) {
        // esc
        case '\e':
            run_loop = 0;
            break;
        // nothing
        case EOF:
            break;
        case KEY_BACKSPACE:
            if (buffer_idx > 0) {
                buffer_idx--;
                buffer[buffer_idx] = '\0';
            }
            break;
        default:
            buffer[buffer_idx] = c;
            buffer_idx++;
            break;
        }

        // render the contents of the buffer to the screen
        // TODO: split by paragraph, sentence, word, char
        // analyze the buffer every time we overflow its size
        // or every time our structure becomes empty
        // the current active portion of the buffer is tacked onto the end
        cy = 0;
        cx = 0;
        for (int i = 0; i < strlen(buffer); i++) {
            move(cy, cx);
            addch(buffer[i]);
            cx++;
        }
        refresh();
    }

    // clean up ncurses
    endwin();

    // test out splitting the buffer into sentences and free allocations
    for (int i = 0; i < number_of_sentences; i++) {
        printf("%s ", sentences[i]);
        free(sentences[i]);
    }
    printf("\n");
    free(sentences);
    return 0;
}
