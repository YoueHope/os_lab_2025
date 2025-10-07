// find_min_max.c
#include "find_min_max.h"
#include <limits.h>

struct MinMax GetMinMax(int *array, unsigned int begin, unsigned int end) {
    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    // Проверка корректности промежутка
    if (begin >= end) {
        min_max.min = min_max.max = 0; // или можно вернуть ошибку
        return min_max;
    }
    
    // Поиск min и max на заданном промежутке [begin, end]
    for (unsigned int i = begin; i < end; i++) {
        if (array[i] < min_max.min) {
            min_max.min = array[i];
        }
        if (array[i] > min_max.max) {
            min_max.max = array[i];
        }
    }
    
    return min_max;
}
