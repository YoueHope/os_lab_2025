#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>

// Общая структура для вычислений
struct FactorialArgs {
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
};

// Общие функции
uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod);
uint64_t Factorial(const struct FactorialArgs *args);
bool ConvertStringToUI64(const char *str, uint64_t *val);

// Функции для работы с серверами
struct Server {
    char ip[255];
    int port;
};

int ReadServers(const char* filename, struct Server** servers);

#endif
