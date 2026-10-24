#include <pthread.h>
#include <stdio.h>
#include <unistd.h> // для функции sleep

pthread_mutex_t mut1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut2 = PTHREAD_MUTEX_INITIALIZER;

void *thread_one(void *arg) { // поток 1 пытается захватить mut1, затем mut2
    pthread_mutex_lock(&mut1);
    printf("Поток 1: захватил mut1\n");
    sleep(1);
    pthread_mutex_lock(&mut2);
    printf("Поток 1: захватил mut2\n");
    pthread_mutex_unlock(&mut2);
    pthread_mutex_unlock(&mut1);
    return NULL;
}

void *thread_two(void *arg) { // поток 2 пытается захватить mut2, затем mut1
    pthread_mutex_lock(&mut2);
    printf("Поток 2: захватил mut2\n");
    sleep(1); // имитация работы, чтобы увеличить вероятность взаимной блокировки
    pthread_mutex_lock(&mut1);
    printf("Поток 2: захватил mut1\n");
    pthread_mutex_unlock(&mut1);
    pthread_mutex_unlock(&mut2);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_one, NULL);
    pthread_create(&t2, NULL, thread_two, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Программа завершена\n");
    return 0;
}