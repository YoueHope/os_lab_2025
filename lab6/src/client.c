#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <getopt.h>

#include "common.h"

struct ThreadData {
    struct Server server;
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
    uint64_t result;
    int success;
    pthread_t thread_id;
};

void* ProcessServer(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;
    
    printf("Thread for server %s:%d started: range [%" PRIu64 " - %" PRIu64 "]\n", 
           data->server.ip, data->server.port, data->begin, data->end);
    
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(data->server.port);
    
    if (inet_pton(AF_INET, data->server.ip, &server.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", data->server.ip);
        data->success = 0;
        return NULL;
    }

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
        fprintf(stderr, "Socket creation failed!\n");
        data->success = 0;
        return NULL;
    }

    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "Connection failed to %s:%d\n", data->server.ip, data->server.port);
        close(sck);
        data->success = 0;
        return NULL;
    }

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &data->begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &data->end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &data->mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
        fprintf(stderr, "Send failed to %s:%d\n", data->server.ip, data->server.port);
        close(sck);
        data->success = 0;
        return NULL;
    }

    char response[sizeof(uint64_t)];
    ssize_t bytes_received = recv(sck, response, sizeof(response), 0);
    if (bytes_received < (ssize_t)sizeof(uint64_t)) {
        fprintf(stderr, "Receive failed from %s:%d (got %zd bytes, expected %zu)\n", 
                data->server.ip, data->server.port, bytes_received, sizeof(uint64_t));
        close(sck);
        data->success = 0;
        return NULL;
    }

    memcpy(&data->result, response, sizeof(uint64_t));
    data->success = 1;
    
    printf("Thread for server %s:%d completed: result = %" PRIu64 "\n", 
           data->server.ip, data->server.port, data->result);
    
    close(sck);
    return NULL;
}

int main(int argc, char **argv) {
    uint64_t k = 0;
    uint64_t mod = 0;
    char servers_file[255] = {'\0'};
    int servers_file_provided = 0;

    while (true) {
        static struct option options[] = {
            {"k", required_argument, 0, 0},
            {"mod", required_argument, 0, 0},
            {"servers", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        if (!ConvertStringToUI64(optarg, &k)) {
                            fprintf(stderr, "Invalid k value: %s\n", optarg);
                            return 1;
                        }
                        break;
                    case 1:
                        if (!ConvertStringToUI64(optarg, &mod)) {
                            fprintf(stderr, "Invalid mod value: %s\n", optarg);
                            return 1;
                        }
                        break;
                    case 2:
                        strncpy(servers_file, optarg, sizeof(servers_file) - 1);
                        servers_file[sizeof(servers_file) - 1] = '\0';
                        servers_file_provided = 1;
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case '?':
                printf("Arguments error\n");
                break;
            default:
                fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (k == 0 || mod == 0 || !servers_file_provided) {
        fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
        return 1;
    }

    struct Server* servers = NULL;
    int servers_num = ReadServers(servers_file, &servers);
    if (servers_num <= 0) {
        fprintf(stderr, "No servers found or error reading servers file\n");
        return 1;
    }

    printf("Found %d servers. Starting parallel computation...\n", servers_num);

    struct ThreadData* thread_data = malloc(sizeof(struct ThreadData) * servers_num);
    if (!thread_data) {
        fprintf(stderr, "Memory allocation failed\n");
        free(servers);
        return 1;
    }
    
    uint64_t segment = k / servers_num;
    uint64_t remainder = k % servers_num;
    uint64_t current_start = 1;
    
    printf("Distributing work for k=%" PRIu64 " mod=%" PRIu64 ":\n", k, mod);
    
    for (int i = 0; i < servers_num; i++) {
        thread_data[i].server = servers[i];
        thread_data[i].begin = current_start;
        
        uint64_t segment_end = current_start + segment - 1;
        if ((uint64_t)i < remainder) {
            segment_end += 1;
        }
        thread_data[i].end = segment_end;
        
        thread_data[i].mod = mod;
        thread_data[i].success = 0;
        
        printf("Server %s:%d gets range [%" PRIu64 " - %" PRIu64 "]\n", 
               servers[i].ip, servers[i].port, thread_data[i].begin, thread_data[i].end);
        
        current_start = thread_data[i].end + 1;
        
        if (pthread_create(&thread_data[i].thread_id, NULL, ProcessServer, &thread_data[i])) {
            fprintf(stderr, "Error creating thread for server %s:%d\n", 
                    servers[i].ip, servers[i].port);
            thread_data[i].success = 0;
        }
    }

    printf("All threads created. Waiting for completion...\n");

    uint64_t total_result = 1;
    int successful_servers = 0;
    
    for (int i = 0; i < servers_num; i++) {
        pthread_join(thread_data[i].thread_id, NULL);
        if (thread_data[i].success) {
            total_result = MultModulo(total_result, thread_data[i].result, mod);
            successful_servers++;
            printf("✓ Server %s:%d completed successfully\n", 
                   servers[i].ip, servers[i].port);
        } else {
            fprintf(stderr, "✗ Server %s:%d failed\n", 
                    servers[i].ip, servers[i].port);
        }
    }

    printf("\n=== RESULTS ===\n");
    printf("Successful servers: %d/%d\n", successful_servers, servers_num);
    printf("Final result: %" PRIu64 "! mod %" PRIu64 " = %" PRIu64 "\n", k, mod, total_result);
    
    free(servers);
    free(thread_data);
    return 0;
}
