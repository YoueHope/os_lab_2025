#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread1_function(void* arg) {
    printf("Поток 1: захватываю mutex1\n");
    pthread_mutex_lock(&mutex1);
    printf("Поток 1: mutex1 захвачен\n");
    
    sleep(2);  // Даем время потоку 2 захватить mutex2
    
    printf("Поток 1: пытаюсь захватить mutex2 (будет DEADLOCK)...\n");
    pthread_mutex_lock(&mutex2);
    printf("Поток 1: mutex2 захвачен\n");
    
    printf("Поток 1: работа завершена\n");
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    return NULL;
}

void* thread2_function(void* arg) {
    printf("Поток 2: захватываю mutex2\n");
    pthread_mutex_lock(&mutex2);
    printf("Поток 2: mutex2 захвачен\n");
    
    sleep(2);  // Даем время потоку 1 захватить mutex1
    
    printf("Поток 2: пытаюсь захватить mutex1 (будет DEADLOCK)...\n");
    pthread_mutex_lock(&mutex1);
    printf("Поток 2: mutex1 захвачен\n");
    
    printf("Поток 2: работа завершена\n");
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    
    printf("=== Демонстрация DEADLOCK ===\n\n");
    printf("Сценарий deadlock:\n");
    printf("1. Поток 1 захватывает mutex1\n");
    printf("2. Поток 2 захватывает mutex2\n");
    printf("3. Поток 1 пытается захватить mutex2 (но он занят)\n");
    printf("4. Поток 2 пытается захватить mutex1 (но он занят)\n");
    printf("5. ОБА ПОТОКА ЗАБЛОКИРОВАНЫ!\n\n");
    
    printf("Запускаем потоки...\n");
    printf("Программа заблокируется через 2-3 секунды.\n");
    printf("Для выхода нажмите Ctrl+C\n\n");
    
    pthread_create(&thread1, NULL, thread1_function, NULL);
    pthread_create(&thread2, NULL, thread2_function, NULL);
    
    // Ожидаем завершения (но программа заблокируется)
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    // Эта строка никогда не выполнится при deadlock
    printf("Программа завершена (этого не должно было произойти)\n");
    
    return 0;
}
