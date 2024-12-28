#include <string.h>
#include <regex.h>
#include "structs.h"

// rendering helpers
void render_italics(Buffer* buffer, int start_index, int* current_state) {
    if (buffer->content[start_index] == '_') {
        int check_index = start_index + 1;
        int buffer_length = strlen(buffer->content);
        while (check_index < buffer_length && buffer->content[check_index] != '_') {
            check_index++;
        }
        if (buffer->content[check_index] == '_') {
            *current_state = 1;
        } else {
            *current_state = 0;
        }
    }
}
