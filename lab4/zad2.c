#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int global = 0;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Użycie: %s <ścieżka do katalogu>\n", argv[0]);
        return 1;
    }

    printf("Nazwa programu: %s\n", argv[0]);

    int local = 0;
    pid_t pid = fork();

    if(pid < 0) {
        perror("Błąd fork");
        return 1;
    }
    else if (pid == 0) {
        printf("Child process\n");

        global++;
        local++;

        printf("child pid = %d, parent pid = %d\n", getpid(), getppid());
        printf("child's local = %d, child's global = %d\n", local, global);

        execl("/bin/ls", "ls", argv[1], NULL);

        perror("Błąd execl");
        return 2;
    }
    else {
        int status;
        waitpid(pid, &status, 0);

        printf("Parent process\n");
        printf("parent pid = %d, child pid = %d\n", getpid(), pid);

        if(WIFEXITED(status)) {
            printf("Child exit code: %d\n", WEXITSTATUS(status));
        } else {
            printf("Child did not terminate normally\n");
        }

        printf("Parent's local = %d, parent's global = %d\n", local, global);
    }

    return 0;
}