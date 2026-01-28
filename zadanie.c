#include "common.h"

int shm1, shm2, sem1, sem2;

void cleanup() {
    shmctl(shm1, IPC_RMID, NULL);
    shmctl(shm2, IPC_RMID, NULL);
    semctl(sem1, 0, IPC_RMID);
    semctl(sem2, 0, IPC_RMID);
    kill(0, SIGTERM);
}

void sig_handler(int sig) {
    (void)sig;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port1_tcp> <port2_udp>\n", argv[0]);
        return 1;
    }
    char *port1 = argv[1];
    char *port2 = argv[2];

    signal(SIGINT, (void (*)(int))cleanup);
    signal(SIGUSR1, sig_handler);

    shm1 = shmget(SHM_KEY_1, sizeof(shared_data_t), IPC_CREAT | 0666);
    shm2 = shmget(SHM_KEY_2, sizeof(shared_data_t), IPC_CREAT | 0666);
    sem1 = semget(SEM_KEY_1, 2, IPC_CREAT | 0666);
    sem2 = semget(SEM_KEY_2, 2, IPC_CREAT | 0666);

    union semun arg;
    unsigned short val1[2] = {1, 0};
    arg.array = val1;
    semctl(sem1, 0, SETALL, arg);

    unsigned short val2[2] = {1, 0};
    arg.array = val2;
    semctl(sem2, 0, SETALL, arg);

    int pipe_to_pr[2];
    pipe(pipe_to_pr);

    int pipe_pr_to_t[2];
    pipe(pipe_pr_to_t);

    pid_t pid_p1, pid_p2, pid_pr, pid_t, pid_s, pid_d, pid_serv1, pid_serv2;

    if ((pid_p1 = fork()) == 0) {
        dup2(pipe_to_pr[1], STDOUT_FILENO);
        close(pipe_to_pr[0]); close(pipe_to_pr[1]);
        close(pipe_pr_to_t[0]); close(pipe_pr_to_t[1]);
        execl("./proc_p1", "proc_p1", NULL);
        perror("Exec P1"); exit(1);
    }

    if ((pid_p2 = fork()) == 0) {
        dup2(pipe_to_pr[1], STDOUT_FILENO);
        close(pipe_to_pr[0]); close(pipe_to_pr[1]);
        close(pipe_pr_to_t[0]); close(pipe_pr_to_t[1]);
        execl("./proc_p2", "proc_p2", NULL);
        perror("Exec P2"); exit(1);
    }

    if ((pid_pr = fork()) == 0) {
        char sp1[10], sp2[10], sfd_r[10], sfd_w[10];
        sprintf(sp1, "%d", pid_p1);
        sprintf(sp2, "%d", pid_p2);
        sprintf(sfd_r, "%d", pipe_to_pr[0]);
        sprintf(sfd_w, "%d", pipe_pr_to_t[1]);

        close(pipe_to_pr[1]);
        close(pipe_pr_to_t[0]);
        
        execl("./proc_pr", "proc_pr", sp1, sp2, sfd_r, sfd_w, NULL);
        perror("Exec Pr"); exit(1);
    }

    close(pipe_to_pr[0]);
    close(pipe_to_pr[1]);

    if ((pid_t = fork()) == 0) {
        dup2(pipe_pr_to_t[0], STDIN_FILENO);
        close(pipe_pr_to_t[0]); close(pipe_pr_to_t[1]);
        execl("./proc_t", "proc_t", NULL);
        perror("Exec T"); exit(1);
    }
    close(pipe_pr_to_t[0]); 
    close(pipe_pr_to_t[1]);

    if ((pid_s = fork()) == 0) {
        char sm1s[10], s1s[10], sm2s[10], s2s[10];
        sprintf(sm1s, "%d", shm1);
        sprintf(s1s, "%d", sem1);
        sprintf(sm2s, "%d", shm2);
        sprintf(s2s, "%d", sem2);
        execl("./proc_s", "proc_s", sm1s, s1s, sm2s, s2s, NULL);
        perror("Exec S"); exit(1);
    }

    if ((pid_serv2 = fork()) == 0) {
        execl("./proc_serv2", "proc_serv2", port2, NULL);
        perror("Exec Serv2"); exit(1);
    }
    sleep(1);

    if ((pid_serv1 = fork()) == 0) {
        execl("./proc_serv1", "proc_serv1", port1, port2, NULL);
        perror("Exec Serv1"); exit(1);
    }
    sleep(1);

    if ((pid_d = fork()) == 0) {
        execl("./proc_d", "proc_d", port1, NULL);
        perror("Exec D"); exit(1);
    }

    int status;
    while (wait(&status) > 0);

    cleanup();
    printf("Zadanie finished.\n");
    return 0;
}