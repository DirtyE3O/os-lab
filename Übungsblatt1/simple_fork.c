#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    fork();

    printf("Hello!\n");
    sleep(1);
}

/*
fork() erstellt einen neuen Prozess (Kind-Prozess), der eine exakte Kopie des aufrufenden Prozesses ist.
Nach dem fork() laufen zwei Prozesse parallel weiter
Beide Prozesse führen den Code nach dem fork()-Aufruf aus.
Beide Prozesse geben "Hello!" aus.
Insgesamt zweimal
*/
