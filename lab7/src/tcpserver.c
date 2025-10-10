#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr

void print_usage(const char *program_name) {
    printf("Usage: %s <PORT> <BUFSIZE>\n", program_name);
    printf("Example: %s 8080 100\n", program_name);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int bufsize = atoi(argv[2]);

    if (bufsize <= 0) {
        printf("Invalid buffer size\n");
        exit(1);
    }

    const size_t kSize = sizeof(struct sockaddr_in);

    int lfd, cfd;
    int nread;
    char *buf = malloc(bufsize);
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;

    if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, kSize);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(lfd, (SADDR *)&servaddr, kSize) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(lfd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    printf("TCP Server listening on port %d\n", port);

    while (1) {
        unsigned int clilen = kSize;

        if ((cfd = accept(lfd, (SADDR *)&cliaddr, &clilen)) < 0) {
            perror("accept");
            exit(1);
        }
        printf("Connection established\n");

        while ((nread = read(cfd, buf, bufsize)) > 0) {
            write(1, buf, nread);
        }

        if (nread == -1) {
            perror("read");
            exit(1);
        }
        close(cfd);
    }

    free(buf);
}
