#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

int main(int argc, char* argv[]) {
    // window and cursor tracking
    int max_x = 0, max_y = 0;
    int left_margin = 0, right_margin = 0;
    int cx = 0, cy = 0;
    int c = '\0';

    // TODO: rework with structs (paragraph struct -> sentence struct -> word struct)
    // we store things in a buffer, then write to the screen and keep a data structure in tact
    // the buffer is small (32) and is analyzed+zeroed upon 'overflow' then stored in the data structure
    char buffer[1024];
    memset(buffer, '\0', sizeof(buffer));
    int buffer_idx = 0;

    int allocated_sentences = 512;
    int sentence_malloc_interval = 512;
    int number_of_sentences = 0;

    char** sentences = malloc(allocated_sentences * sizeof(char*));
    if (!sentences) {
        printf("Could not allocate memory!");
        exit(1);
    }
    memset(sentences, 0, allocated_sentences * sizeof(char*));

    for (int i = 0; i < allocated_sentences; i++) {
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
        timeout(1000);
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

        // break contents of buffer into sentences
        number_of_sentences = 0;
        int char_idx = 0;
        for (int i = 0; i < strlen(buffer); i++) {
            char curr = buffer[i];
            char next = buffer[i + 1];
            switch (buffer[i]) {
            case '.':
                if (next == '.' || next == '?' || next == '!') {
                    sentences[number_of_sentences][char_idx] = curr;
                    sentences[number_of_sentences][char_idx + 1] = '\0';
                    char_idx++;
                    break;
                }
            case '?':
            case '!':
                sentences[number_of_sentences][char_idx] = curr;
                sentences[number_of_sentences][char_idx + 1] = '\0';
                number_of_sentences++;
                char_idx = 0;
                break;
            case ' ':
                if (char_idx != 0) {
                    sentences[number_of_sentences][char_idx] = curr;
                    sentences[number_of_sentences][char_idx + 1] = '\0';
                    char_idx++;
                }
                break;
            default:
                sentences[number_of_sentences][char_idx] = curr;
                sentences[number_of_sentences][char_idx + 1] = '\0';
                char_idx++;
                break;
            }
        }

        // render the contents of the buffer to the screen
        erase();
        left_margin = max_x / 2 - 36;
        right_margin = max_x / 2 + 36;
        cy = 0;
        cx = left_margin;
        int length_of_buffer = strlen(buffer);
        // TODO: enhance these 3 lines. this is way too naive and leads to layout alterations
        // use words per line as a better metric. Likely will have to reflow text...
        // do we build a structure for rendering?
        int starting_idx = 0;
        int number_of_visible_lines = length_of_buffer / 72;
        if (number_of_visible_lines > max_y - 2) {
            starting_idx = (number_of_visible_lines - (max_y - 2)) * 72;
        }
        for (int i = starting_idx; i < length_of_buffer; i++) {
            int temp_cx = cx;
            for (int k = i; buffer[k] != ' ' && buffer[k] != '\0' && buffer[k] != '\n'; k++) {
                temp_cx++;
                if (temp_cx > right_margin && k - i < 16) {
                    cx = left_margin;
                    cy++;
                    break;
                }
            }
            move(cy, cx);
            if (cx == left_margin && buffer[i] == ' ') {
                ;
            } else {
                addch(buffer[i]);
            }
            if (buffer[i] == '\n') {
                cx = left_margin;
                cy++;
            } else {
                cx++;
            }
            if (cx > right_margin) {
                cx = left_margin;
                cy++;
            }
        }
        refresh();
    }

    // clean up ncurses
    endwin();

    // test out splitting the buffer into sentences and free allocations
    for (int i = 0; i < allocated_sentences; i++) {
        if (i < number_of_sentences + 1) {
            printf("%2i| %s\n", i + 1, sentences[i]);
        }
        free(sentences[i]);
    }
    printf("\n");
    free(sentences);
    return 0;
}
