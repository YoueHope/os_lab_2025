// parallel_min_max.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/wait.h>
#include <sys/time.h>
#include <getopt.h>
#include <signal.h>
#include <limits.h>
#include "find_min_max.h"
#include "utils.h"

// Глобальная переменная для хранения PID дочерних процессов
pid_t *child_pids = NULL;
int num_children = 0;
int kill(pid_t pid, int sig);

// Обработчик сигнала для таймаута
void timeout_handler(int sig) {
    if (sig == SIGALRM) {
        printf("Timeout reached! Killing child processes...\n");
        for (int i = 0; i < num_children; i++) {
            if (child_pids[i] > 0) {
                kill(child_pids[i], SIGKILL);
                printf("Killed child process with PID: %d\n", child_pids[i]);
            }
        }
    }
}

int main(int argc, char **argv) {
    int timeout = 0; // 0 означает отсутствие таймаута
    int seed = 0;
    int array_size = 0;
    int pnum = 0;
    int pipefd[2];
    int show = 0;
    
    // Парсинг аргументов командной строки
    static struct option long_options[] = {
        {"seed", required_argument, 0, 's'},
        {"array_size", required_argument, 0, 'a'},
        {"pnum", required_argument, 0, 'p'},
        {"timeout", required_argument, 0, 't'},
        {"show", no_argument, 0, 'w'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "s:a:p:t:w", long_options, &option_index)) != -1) {
        switch (c) {
            case 's':
                seed = atoi(optarg);
                break;
            case 'a':
                array_size = atoi(optarg);
                break;
            case 'p':
                pnum = atoi(optarg);
                break;
            case 't':
                timeout = atoi(optarg);
                break;
            case 'w':
                show = 1;
                break;
            case '?':
                break;
            default:
                printf("Usage: %s --seed <num> --array_size <num> --pnum <num> [--timeout <sec>] [--show]\n", argv[0]);
                return 1;
        }
    }
    
    if (seed == 0 || array_size == 0 || pnum == 0) {
        printf("Usage: %s --seed <num> --array_size <num> --pnum <num> [--timeout <sec>] [--show]\n", argv[0]);
        return 1;
    }
    
    // Выделяем память для хранения PID дочерних процессов
    child_pids = malloc(pnum * sizeof(pid_t));
    if (child_pids == NULL) {
        perror("malloc");
        return 1;
    }
    
    // Устанавливаем обработчик сигнала таймаута
    if (timeout > 0) {
        signal(SIGALRM, timeout_handler);
        alarm(timeout);
        printf("Timeout set to %d seconds\n", timeout);
    }
    
    // Генерация массива
    int *array = malloc(array_size * sizeof(int));
    GenerateArray(array, array_size, seed);
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        free(array);
        free(child_pids);
        return 1;
    }
    
    // Разделение работы между процессами
    int active_child_processes = 0;
    int segment_size = array_size / pnum;
    
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    
    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        if (child_pid >= 0) {
            if (child_pid == 0) {
                // Дочерний процесс
                struct MinMax min_max;
                unsigned int begin = i * segment_size;
                unsigned int end = (i == pnum - 1) ? array_size : (i + 1) * segment_size;
                
                min_max = GetMinMax(array, begin, end);
                
                close(pipefd[0]); // Закрываем чтение
                write(pipefd[1], &min_max, sizeof(struct MinMax));
                close(pipefd[1]);
                
                free(array);
                free(child_pids);
                exit(0);
            } else {
                // Родительский процесс
                child_pids[num_children++] = child_pid;
                active_child_processes++;
            }
        } else {
            perror("fork");
            free(array);
            free(child_pids);
            return 1;
        }
    }
    
    close(pipefd[1]); // Закрываем запись в родительском процессе
    
    // Чтение результатов из дочерних процессов
    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;
    
    for (int i = 0; i < active_child_processes; i++) {
        struct MinMax local_min_max;
        read(pipefd[0], &local_min_max, sizeof(struct MinMax));
        
        if (local_min_max.min < min_max.min) {
            min_max.min = local_min_max.min;
        }
        if (local_min_max.max > min_max.max) {
            min_max.max = local_min_max.max;
        }
    }
    
    close(pipefd[0]);
    
    // Ожидание завершения всех дочерних процессов
    for (int i = 0; i < active_child_processes; i++) {
        int status;
        waitpid(child_pids[i], &status, 0);
        
        if (WIFSIGNALED(status)) {
            printf("Child process %d was terminated by signal %d\n", child_pids[i], WTERMSIG(status));
        }
    }
    
    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);
    
    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;
    
    // Отмена таймаута, если он был установлен и мы дошли сюда
    if (timeout > 0) {
        alarm(0);
    }
    
    // Вывод результатов
    if (show) {
        printf("Generated array:\n");
        for (int i = 0; i < array_size; i++) {
            printf("%d ", array[i]);
        }
        printf("\n");
    }
    
    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %f ms\n", elapsed_time);
    
    free(array);
    free(child_pids);
    return 0;
}

