#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t child_pid = fork();

    if (child_pid > 0) {
        // Родительский процесс не вызывает wait(), а просто засыпает.
        printf("Parent (PID %d) is sleeping. Open another terminal and run 'ps aux | grep Z'\n", getpid());
        sleep(20); 
        printf("Parent woke up and finished.\n");
    } else if (child_pid == 0) {
        // Дочерний процесс мгновенно умирает, становясь зомби.
        printf("Child (PID %d) exiting immediately and becoming a zombie...\n", getpid());
        exit(0);
    } else {
        perror("fork failed");
        return 1;
    }

    return 0;
}