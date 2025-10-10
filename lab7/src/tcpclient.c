#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr
#define SIZE sizeof(struct sockaddr_in)

void print_usage(const char *program_name) {
    printf("Usage: %s <IP> <PORT> <BUFSIZE>\n", program_name);
    printf("Example: %s 127.0.0.1 8080 100\n", program_name);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        print_usage(argv[0]);
        exit(1);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int bufsize = atoi(argv[3]);

    if (bufsize <= 0) {
        printf("Invalid buffer size\n");
        exit(1);
    }

    int fd;
    int nread;
    char *buf = malloc(bufsize);
    struct sockaddr_in servaddr;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creating");
        exit(1);
    }

    memset(&servaddr, 0, SIZE);
    servaddr.sin_family = AF_INET;

    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        perror("bad address");
        exit(1);
    }

    servaddr.sin_port = htons(port);

    if (connect(fd, (SADDR *)&servaddr, SIZE) < 0) {
        perror("connect");
        exit(1);
    }

    write(1, "Input message to send\n", 22);
    while ((nread = read(0, buf, bufsize)) > 0) {
        if (write(fd, buf, nread) < 0) {
            perror("write");
            exit(1);
        }
    }

    free(buf);
    close(fd);
    exit(0);
}
