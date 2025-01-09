#include <stdio.h>
#include <string.h>
#include "structs.h"
#include "buffers.h"
#include "helpers.h"
#include "stringop.h"

int load_file(char* filepath, struct Line** head, int* total_lines) {
    char c;
    int i = 0;
    int bytes_loaded = 0;
    struct Line* current_line = *head;
    FILE* file;

    file = fopen(filepath, "rb");
    if (NULL == file) {
        copy_string(filepath, "untitled.txt", 12);
        return -1;
    }

    /* reset line count */
    *total_lines = 1;
    while ((c = fgetc(file)) != EOF) {
        insert_into_buffer(current_line->buffer, i, c);
        i++;
        bytes_loaded++;
        
        if (c == '\n') {
            Buffer* new_buffer = NULL;
            init_buffer(&new_buffer);
            add_line_to_end(head, new_buffer);
            current_line = current_line->next;
            *total_lines += 1;
            i = 0;
        }
    }

    /* delete last-loaded char if it is \n */
    if (current_line->buffer->content[i] == '\n') {
        current_line->buffer->content[i] = '\0';
    }
    /* delete final line if it is empty */
    if (current_line->buffer->content[0] == '\0') {
        remove_line(head, current_line);
        *total_lines -= 1;
    }

    fclose(file);

    return bytes_loaded;
}

int save_file(char* filepath, struct Line* head) {
    int bytes_written = 0;
    struct Line* current_line = head;
    FILE* file;

    file = fopen(filepath, "wb");
    if (NULL == file) {
        fprintf(stderr, "%s", "ERR: could not write file!");
        return -1;
    }

    while (NULL != current_line) {
        int i;
        char* content = current_line->buffer->content;
        for (i = 0; i < strlen(content) && content[i] != '\0'; i++) {
            fputc(content[i], file);
            bytes_written++;
        }
        current_line = current_line->next;
    }

    /* add a spare newline for cleanliness' sake */
    fputc('\n', file);

    fclose(file);
    return bytes_written;
}
