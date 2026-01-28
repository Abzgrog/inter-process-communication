#include "common.h"

#define BUFFER_SIZE 1024

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    char buffer[BUFFER_SIZE];
    const int _PORT = atoi(argv[1]);

    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket id create"); 
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  
    server_addr.sin_port = htons(_PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(1);
    }

    printf("Serv2 (UDP) started on port %d. Waiting for data...\n", _PORT);

    while (1) {
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                             (struct sockaddr*)&client_addr, &addr_len);
        
        if (n < 0) {
            perror("recvfrom error");
            continue;
        }

        buffer[n] = '\0';

        FILE *fp = fopen(FILE_OUT, "a");
        if (fp == NULL) {
            perror("File open error");
            continue;
        }

        fprintf(fp, "%s\n", buffer);
        
        fclose(fp);

        printf("[Serv2] Saved: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}