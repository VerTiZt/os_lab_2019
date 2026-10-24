#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <pthread.h>

// общие данные и мьютекс для синхронизации
long long total = 1;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

// структура аргументов для каждого потока
struct FactorialArgs {
    int begin;
    int end;
    int mod;
};

// функция потока для вычисления части произведения
void *ThreadFactorial(void *args) {
    struct FactorialArgs *f_args = (struct FactorialArgs *)args;
    long long local_res = 1;

    for (int i = f_args->begin; i <= f_args->end; i++) {
        local_res = (local_res * i) % f_args->mod;
    }

    // критическая секция: защищенная запись в общий результат
    pthread_mutex_lock(&mut);
    total = (total * local_res) % f_args->mod;
    pthread_mutex_unlock(&mut);

    return NULL;
}

int main(int argc, char **argv) {
    int k = -1;
    int pnum = -1;
    int mod = -1;

    static struct option options[] = {
        {"pnum", required_argument, 0, 0},
        {"mod", required_argument, 0, 0},
        {"k", required_argument, 0, 'k'},
        {0, 0, 0, 0}
    };
    // обработка аргументов командной строки
    while (true) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "k:", options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        pnum = atoi(optarg);
                        break;
                    case 1:
                        mod = atoi(optarg);
                        break;
                }
                break;
            case 'k':
                k = atoi(optarg);
                break;
            default:
                printf("неизвестный параметр\n");
                return 1;
        }
    }
    // проверка корректности введенных параметров
    if (k < 0 || pnum <= 0 || mod <= 0) {
        printf("использование: %s -k <num> --pnum=<num> --mod=<num>\n", argv[0]);
        return 1;
    }
    // если k равно 0, результат всегда 1
    if (k == 0) {
        printf("0! mod %d = %lld\n", mod, 1LL % mod);
        return 0;
    }

    // если потоков запрошено больше чем чисел, ограничиваем их количество
    if (pnum > k) {
        pnum = k;
    }

    pthread_t threads[pnum];
    struct FactorialArgs args[pnum];

    int chunk = k / pnum;

    // создание и запуск рабочих потоков
    for (int i = 0; i < pnum; i++) {
        args[i].begin = i * chunk + 1;
        args[i].end = (i == pnum - 1) ? k : (i + 1) * chunk;
        args[i].mod = mod;
        // проверка на успешное создание потока
        if (pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    // ожидание завершения работы каждого потока
    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL); // если убрать, значение total может быть некорректным, так как потоки могут не успеть завершить работу
    }

    printf("%d! mod %d = %lld\n", k, mod, total);

    return 0;
}