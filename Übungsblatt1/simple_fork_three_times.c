#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    for (int i = 0; i < 3; i++) {
        fork();
    }
    printf("Hello!\n");
    sleep(1);
}

/*
Nach dem 1. fork() gibt es 2 Prozesse.
Beide führen den Code weiter aus.

Nach dem 2. fork() ruft jeder der 2 Prozesse fork() auf
Jeder erstellt einen weiteren Prozess.
Jetzt gibt es 4 Prozesse.

Nach dem 3. fork() ruft jeder der 4 Prozesse fork() auf
Jeder erstellt einen weiteren Prozess.
Jetzt gibt es 8 Prozesse.

Insgesamt 2^3 = 8 Prozesse.
*/
