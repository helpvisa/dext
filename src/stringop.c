void copy_string(char* to, char* from, int limit) {
    int i;

    for (i = 0; i < limit && from[i] != '\0'; i++) {
        to[i] = from[i];
    }
    to[i] = '\0';
}
