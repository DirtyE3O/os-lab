#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    // fork a process
    pid_t pid = fork();
    if (pid == 0) {
        // child process
        pid = getpid();
        pid_t ppid = getppid();
        printf("This is child process (pid=%d) with parent %d.\n", pid, ppid);
        // child sleeps for 5 seconds
        sleep(5);
        exit(0);
    }
    else if (pid > 1) {
        // parent process
        pid = getpid();
        printf("This is the parent process with process id %d.\n", pid);
        // wait for child to finish
        pid = wait(NULL);
        if (pid == -1) {
            printf("Wait failed.\n");
            exit(1);
        }
        printf("Child %d finished.\n", pid);
        exit(0);
    }
    else {
        printf("Fork failed.\n");
        exit(1);
    }
}

/*

Der Unterschied entsteht dadurch, dass fork() verschiedene Rückgabewerte liefert:
Im Kindprozess gibt fork() 0 zurück
Im Elternprozess gibt fork()  die PID des Kindprozesses zurück
Bei Fehler gibt fork()  -1 zurück.
Durch die Frage (if (pid == 0)) können die Prozesse gesteuert werden.


getpid() liefert die Prozess-ID des aktuell laufenden Prozesses.
getppid() liefert die Prozess-ID des Elternprozesses.
wait() lässt den Elternprozess warten, bis ein Kindprozess beendet ist.

wait() liefert die PID des beendeten Kindes als Rückgabewert zurück.
wait() legt den Beendigungsstatus des Kindes in einer Variablen im Speicher ab, wenn man einen Zeiger darauf übergibt.
z.B.
int status;
pid_t child_pid = wait(&status);

*/