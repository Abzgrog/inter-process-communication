#include "common.h"

int main() {
    int shmid = shmget(SHM_KEY_1, sizeof(shared_data_t), 0666);
    int semid = semget(SEM_KEY_1, 2, 0666);
    
    if (shmid < 0 || semid < 0) {
        perror("T: Resource get failed");
        exit(1);
    }

    shared_data_t *mem = (shared_data_t *)shmat(shmid, NULL, 0);
    if (mem == (void*)-1) { perror("T: shmat"); exit(1); }

    char buffer[MAX_BUFFER];

    while (scanf("%s", buffer) != EOF) {
        
        struct sembuf wait_op = {0, -1, 0};
        if (semop(semid, &wait_op, 1) < 0) break;

        strcpy(mem->text, buffer);

        struct sembuf signal_op = {1, 1, 0};
        if (semop(semid, &signal_op, 1) < 0) break;
    }

    shmdt(mem);
    return 0;
}