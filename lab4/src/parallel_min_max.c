#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include <signal.h>

#include "find_min_max.h"
#include "utils.h"

// Глобальные переменные для доступа из обработчика сигналов
#include <signal.h>

pid_t *child_pids = NULL;
int pnum_global = 0;
volatile sig_atomic_t timeout_happened = 0; // флаг таймаута

void handle_alarm(int sig) {
    timeout_happened = 1;
    printf("\n[Timeout] Killing child processes...\n");
    for (int i = 0; i < pnum_global; i++) {
        if (child_pids[i] > 0) {
            kill(child_pids[i], SIGKILL);
        }
    }
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = -1; // Новый параметр
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"timeout", required_argument, 0, 0}, // Добавлено
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
                printf("Seed must be a positive number\n");
                return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
                printf("Array size must be a positive number\n");
                return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
                printf("Number of processes must be a positive number\n");
                return 1;
            }
            break;
          case 3:
            with_files = true;
            break;
          case 4: // Обработка timeout
            timeout = atoi(optarg);
            if (timeout <= 0) {
                printf("Timeout must be a positive number\n");
                return 1;
            }
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"]\n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int pipefd[2];
  if (!with_files) {
      if (pipe(pipefd) == -1) {
          perror("Pipe failed");
          return 1;
      }
  }

  // Выделяем память под массив PID
  child_pids = malloc(sizeof(pid_t) * pnum);
  pnum_global = pnum;

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      active_child_processes += 1;
      child_pids[i] = child_pid; // Сохраняем PID дочернего процесса

      if (child_pid == 0) { 
        int chunk_size = array_size / pnum;
        int begin = i * chunk_size;
        int end = (i == pnum - 1) ? array_size : (i + 1) * chunk_size;
        // сюда вставляю таймер
        // sleep(3);
        struct MinMax local_min_max = GetMinMax(array, begin, end);

        if (with_files) {
          char filename[256];
          sprintf(filename, "tmp_data_%d.txt", i);
          FILE *fp = fopen(filename, "w");
          fprintf(fp, "%d %d\n", local_min_max.min, local_min_max.max);
          fclose(fp);
        } else {
          write(pipefd[1], &local_min_max.min, sizeof(int));
          write(pipefd[1], &local_min_max.max, sizeof(int));
        }
        return 0; 
      }
    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  // Если задан таймаут, настраиваем сигнал и запускаем таймер
  if (timeout > 0) {
      signal(SIGALRM, handle_alarm);
      alarm(timeout);
  }

  // Неблокирующее ожидание завершения дочерних процессов
  while (active_child_processes > 0) {
      int status;
      // WNOHANG заставляет waitpid возвращать 0 немедленно, если процесс еще работает
      pid_t result = waitpid(-1, &status, WNOHANG);
      if (result > 0) {
          active_child_processes -= 1;
      }
  }

  // Отключаем таймер, если процессы успели завершиться сами
  if (timeout > 0) {
      alarm(0);
  }

  // если процессы были убиты по таймауту, прерываем программу
  if (timeout_happened) {
      printf("Calculation aborted due to timeout.\n");
      free(child_pids);
      free(array);
      return 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char filename[256];
      sprintf(filename, "tmp_data_%d.txt", i);
      FILE *fp = fopen(filename, "r");
      if (fp) {
          fscanf(fp, "%d %d", &min, &max);
          fclose(fp);
          remove(filename); 
      }
    } else {
      read(pipefd[0], &min, sizeof(int));
      read(pipefd[0], &max, sizeof(int));
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(child_pids);
  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}