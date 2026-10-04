#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("usage: %s seed arraysize\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid == 0) {
        // заменяем дочерний процесс на нашу готовую программу
        char *args[] = {"./sequential_min_max", argv[1], argv[2], NULL};
        execv("./sequential_min_max", args);
        
        // сюда код дойдет, только если execv не сможет найти файл
        perror("exec failed");
        return 1;
    } else if (pid > 0) {
        // родительский процесс ждет, пока отработает программа
        wait(NULL);
    } else {
        perror("fork failed");
        return 1;
    }

    return 0;
}