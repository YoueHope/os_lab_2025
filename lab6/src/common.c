#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
    uint64_t result = 0;
    a = a % mod;
    while (b > 0) {
        if (b % 2 == 1)
            result = (result + a) % mod;
        a = (a * 2) % mod;
        b /= 2;
    }
    return result % mod;
}

uint64_t Factorial(const struct FactorialArgs *args) {
    uint64_t ans = 1;
    
    for (uint64_t i = args->begin; i <= args->end; i++) {
        ans = MultModulo(ans, i, args->mod);
    }
    
    return ans;
}

bool ConvertStringToUI64(const char *str, uint64_t *val) {
    char *end = NULL;
    unsigned long long i = strtoull(str, &end, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "Out of uint64_t range: %s\n", str);
        return false;
    }
    if (errno != 0)
        return false;
    *val = i;
    return true;
}

int ReadServers(const char* filename, struct Server** servers) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open servers file");
        return -1;
    }

    int capacity = 10;
    int count = 0;
    *servers = malloc(sizeof(struct Server) * capacity);
    if (!*servers) {
        fclose(file);
        return -1;
    }
    
    char line[255];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        if (count >= capacity) {
            capacity *= 2;
            struct Server* new_servers = realloc(*servers, sizeof(struct Server) * capacity);
            if (!new_servers) {
                free(*servers);
                fclose(file);
                return -1;
            }
            *servers = new_servers;
        }
        
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            size_t ip_len = strlen(line);
            if (ip_len >= sizeof((*servers)[count].ip)) {
                ip_len = sizeof((*servers)[count].ip) - 1;
            }
            memcpy((*servers)[count].ip, line, ip_len);
            (*servers)[count].ip[ip_len] = '\0';
            (*servers)[count].port = atoi(colon + 1);
            count++;
        }
    }
    
    fclose(file);
    return count;
}
