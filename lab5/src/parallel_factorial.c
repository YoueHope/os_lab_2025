#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <getopt.h>

// Структура для передачи данных в потоки
typedef struct {
    long long start;      // Начало диапазона
    long long end;        // Конец диапазона
    long long mod;        // Модуль
    long long partial_result; // Частичный результат потока
} thread_data_t;

// Глобальные переменные
long long global_result = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Функция для вычисления произведения в диапазоне по модулю
void* compute_partial_factorial(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    data->partial_result = 1;
    
    printf("Поток: вычисляю произведение от %lld до %lld\n", 
           data->start, data->end);
    
    for (long long i = data->start; i <= data->end; i++) {
        data->partial_result = (data->partial_result * i) % data->mod;
    }
    
    printf("Поток: частичный результат = %lld\n", data->partial_result);
    
    // Синхронизированное обновление глобального результата
    pthread_mutex_lock(&mutex);
    global_result = (global_result * data->partial_result) % data->mod;
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

// Функция для разбора аргументов командной строки
void parse_arguments(int argc, char* argv[], long long* k, int* pnum, long long* mod) {
    static struct option long_options[] = {
        {"k", required_argument, 0, 'k'},
        {"pnum", required_argument, 0, 'p'},
        {"mod", required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    // Установка значений по умолчанию
    *k = 10;
    *pnum = 4;
    *mod = 1000000007;
    
    while ((opt = getopt_long(argc, argv, "k:p:m:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'k':
                *k = atoll(optarg);
                if (*k < 0) {
                    fprintf(stderr, "Ошибка: k должно быть неотрицательным\n");
                    exit(1);
                }
                break;
            case 'p':
                *pnum = atoi(optarg);
                if (*pnum <= 0) {
                    fprintf(stderr, "Ошибка: pnum должно быть положительным\n");
                    exit(1);
                }
                break;
            case 'm':
                *mod = atoll(optarg);
                if (*mod <= 0) {
                    fprintf(stderr, "Ошибка: mod должно быть положительным\n");
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, "Использование: %s -k <число> --pnum=<потоки> --mod=<модуль>\n", argv[0]);
                exit(1);
        }
    }
}

int main(int argc, char* argv[]) {
    long long k, mod;
    int pnum;
    
    // Парсинг аргументов
    parse_arguments(argc, argv, &k, &pnum, &mod);
    
    printf("Вычисление %lld! mod %lld с использованием %d потоков\n", k, mod, pnum);
    
    // Особые случаи
    if (k == 0 || k == 1) {
        printf("Результат: 1\n");
        return 0;
    }
    
    // Если потоков больше чем чисел для вычисления
    if (pnum > k) {
        pnum = k;
        printf("Уменьшаем количество потоков до %d (больше чем чисел для вычисления)\n", pnum);
    }
    
    // Создание потоков и данных для них
    pthread_t* threads = malloc(pnum * sizeof(pthread_t));
    thread_data_t* thread_data = malloc(pnum * sizeof(thread_data_t));
    
    if (!threads || !thread_data) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        exit(1);
    }
    
    // Распределение работы между потоками
    long long numbers_per_thread = k / pnum;
    long long remainder = k % pnum;
    long long current_start = 1;
    
    for (int i = 0; i < pnum; i++) {
        thread_data[i].start = current_start;
        thread_data[i].end = current_start + numbers_per_thread - 1;
        
        // Распределяем остаток по первым потокам
        if (i < remainder) {
            thread_data[i].end++;
        }
        
        thread_data[i].mod = mod;
        current_start = thread_data[i].end + 1;
        
        printf("Поток %d: диапазон [%lld, %lld]\n", 
               i, thread_data[i].start, thread_data[i].end);
    }
    
    // Запуск потоков
    for (int i = 0; i < pnum; i++) {
        if (pthread_create(&threads[i], NULL, compute_partial_factorial, &thread_data[i]) != 0) {
            fprintf(stderr, "Ошибка создания потока %d\n", i);
            exit(1);
        }
    }
    
    // Ожидание завершения всех потоков
    for (int i = 0; i < pnum; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Ошибка ожидания потока %d\n", i);
            exit(1);
        }
    }
    
    // Вывод результата
    printf("\nФинальный результат: %lld! mod %lld = %lld\n", k, mod, global_result);
    
    // Проверка (последовательное вычисление для верификации)
    long long sequential_result = 1;
    for (long long i = 1; i <= k; i++) {
        sequential_result = (sequential_result * i) % mod;
    }
    printf("Проверка (последовательно): %lld! mod %lld = %lld\n", k, mod, sequential_result);
    
    if (global_result == sequential_result) {
        printf("✓ Результаты совпадают!\n");
    } else {
        printf("✗ Ошибка: результаты не совпадают!\n");
    }
    
    // Освобождение ресурсов
    free(threads);
    free(thread_data);
    pthread_mutex_destroy(&mutex);
    
    return 0;
}
