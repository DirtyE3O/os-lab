#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {

    int n = atoi(argv[1]);

    pid_t* pids = malloc(n * sizeof(pid_t));
    if (pids == NULL) {
        perror("malloc");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        const pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(pids);
            return 1;
        }
        else if (pid == 0) {
            // child
            printf("This is child process %d (pid=%d).\n", i + 1, getpid());
            sleep(5);
            _exit(0);
        }
        else {
            pids[i] = pid;
        }
    }

    for (int k = 0; k < n; k++) {
        pid_t finished = pids[k];
        int status;
        waitpid(finished, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Child process finished with process id %d.\n", finished);
        }
    }

    free(pids);
    printf("This is the parent process with process id %d.\n", getpid());
    printf("Parent process finished.\n");
    return 0;
}
