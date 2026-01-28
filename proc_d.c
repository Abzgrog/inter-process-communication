#include "common.h"

int main(int argc, char *argv[]) {
    if (argc < 2) exit(1);
    int port = atoi(argv[1]);

    int shmid = shmget(SHM_KEY_2, sizeof(shared_data_t), 0666);
    int semid = semget(SEM_KEY_2, 2, 0666);
    
    shared_data_t *mem = (shared_data_t *)shmat(shmid, NULL, 0);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    sleep(1); 
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("D: connect failed");
        exit(1);
    }

    char buffer[MAX_BUFFER];

    while (1) {
        struct sembuf wait_op = {1, -1, 0};
        if (semop(semid, &wait_op, 1) < 0) break;

        strcpy(buffer, mem->text);

        struct sembuf signal_op = {0, 1, 0};
        if (semop(semid, &signal_op, 1) < 0) break;

        send(sock, buffer, strlen(buffer) + 1, 0);
    }

    close(sock);
    shmdt(mem);
    return 0;
}