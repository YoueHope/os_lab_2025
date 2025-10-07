#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>
#include <pthread.h>
#include "sum_lib.h"
#include "utils.h"

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)Sum(sum_args);
}

int main(int argc, char **argv) {
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;
  int show = 0;
  
  // Парсинг аргументов командной строки
  static struct option long_options[] = {
    {"threads_num", required_argument, 0, 't'},
    {"array_size", required_argument, 0, 'a'},
    {"seed", required_argument, 0, 's'},
    {"show", no_argument, 0, 'w'},
    {0, 0, 0, 0}
  };
  
  int option_index = 0;
  int c;
  while ((c = getopt_long(argc, argv, "t:a:s:w", long_options, &option_index)) != -1) {
    switch (c) {
      case 't':
        threads_num = atoi(optarg);
        break;
      case 'a':
        array_size = atoi(optarg);
        break;
      case 's':
        seed = atoi(optarg);
        break;
      case 'w':
        show = 1;
        break;
      case '?':
        break;
      default:
        printf("Usage: %s --threads_num <num> --array_size <num> --seed <num> [--show]\n", argv[0]);
        return 1;
    }
  }
  
  if (threads_num == 0 || array_size == 0) {
    printf("Usage: %s --threads_num <num> --array_size <num> --seed <num> [--show]\n", argv[0]);
    return 1;
  }
  
  // Генерация массива (не входит в замер времени)
  int *array = malloc(sizeof(int) * array_size);
  if (array == NULL) {
    perror("malloc");
    return 1;
  }
  
  GenerateArray(array, array_size, seed);
  
  // Вывод массива если указана опция --show
  if (show) {
    printf("Generated array (%d elements):\n", array_size);
    for (uint32_t i = 0; i < array_size; i++) {
      printf("%d ", array[i]);
      if ((i + 1) % 10 == 0) printf("\n"); // 10 чисел в строке для читаемости
    }
    printf("\n\n");
  }
  
  pthread_t threads[threads_num];
  struct SumArgs args[threads_num];
  
  // Вывод информации о разделении работы между потоками
  if (show) {
    printf("Work distribution among %d threads:\n", threads_num);
    int segment_size = array_size / threads_num;
    for (uint32_t i = 0; i < threads_num; i++) {
      int begin = i * segment_size;
      int end = (i == threads_num - 1) ? array_size : (i + 1) * segment_size;
      printf("Thread %d: indices [%d, %d) - %d elements\n", i, begin, end, end - begin);
    }
    printf("\n");
  }
  
  // Начало замера времени
  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  
  // Разделение работы между потоками
  int segment_size = array_size / threads_num;
  
  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = i * segment_size;
    args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * segment_size;
    
    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
      printf("Error: pthread_create failed!\n");
      free(array);
      return 1;
    }
  }
  
  // Сбор результатов
  int total_sum = 0;
  int partial_sums[threads_num];
  
  for (uint32_t i = 0; i < threads_num; i++) {
    int sum = 0;
    pthread_join(threads[i], (void **)&sum);
    partial_sums[i] = sum;
    total_sum += sum;
  }
  
  // Конец замера времени
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);
  
  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;
  
  // Вывод частичных сумм если указана опция --show
  if (show) {
    printf("Partial sums from each thread:\n");
    for (uint32_t i = 0; i < threads_num; i++) {
      printf("Thread %d sum: %d\n", i, partial_sums[i]);
    }
    printf("\n");
  }
  
  free(array);
  printf("Total sum: %d\n", total_sum);
  printf("Elapsed time: %f ms\n", elapsed_time);
  
  return 0;
}
