#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

void print_usage(const char *program_name) {
    printf("Usage: %s <IP> <PORT> <BUFSIZE>\n", program_name);
    printf("Example: %s 127.0.0.1 8080 1024\n", program_name);
}

int main(int argc, char **argv) {
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

    int sockfd, n;
    char *sendline = malloc(bufsize);
    char *recvline = malloc(bufsize + 1);
    struct sockaddr_in servaddr;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) < 0) {
        perror("inet_pton problem");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket problem");
        exit(1);
    }

    write(1, "Enter string\n", 13);

    while ((n = read(0, sendline, bufsize)) > 0) {
        if (sendto(sockfd, sendline, n, 0, (SADDR *)&servaddr, SLEN) == -1) {
            perror("sendto problem");
            exit(1);
        }

        if (recvfrom(sockfd, recvline, bufsize, 0, NULL, NULL) == -1) {
            perror("recvfrom problem");
            exit(1);
        }

        recvline[n] = '\0';
        printf("REPLY FROM SERVER: %s\n", recvline);
    }

    free(sendline);
    free(recvline);
    close(sockfd);
    return 0;
}
