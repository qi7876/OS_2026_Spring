#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        char *args[] = {"./a", NULL};

        execv("./a", args);
        perror("execv");
        _exit(1);
    }

    if (waitpid(pid, NULL, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    printf("Bye\n");
    return 0;
}
