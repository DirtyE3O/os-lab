#include <stdio.h>

int main(void) {
    char s[200];


    printf("Gib einen Satz ein: ");
    fgets(s, sizeof(s), stdin);

    for (char *p = s; *p != '\0'; p++) {
        if (*p == ' ') {
            putchar('\n');
        } 
        else if (*p != '\n') {
            putchar(*p);
        }
    }

    return 0;
}