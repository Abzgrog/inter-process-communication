#include "common.h"

volatile sig_atomic_t ready_to_send = 0;

void my_handler(int signum) {
    (void)signum;
    ready_to_send = 1;
}

int main() {
    FILE *fp;
    char buffer[MAX_BUFFER];

    if (signal(SIGUSR1, my_handler) == SIG_ERR) {
        perror("Signal error");
        exit(1);
    }

    fp = fopen(FILE_P2, "r");
    if (fp == NULL) {
        perror("Error opening file p2.txt");
        exit(1);
    }

    while (1) {
        pause();

        if (ready_to_send) {
            ready_to_send = 0;

            if (fscanf(fp, "%s", buffer) != EOF) {
                printf("%s\n", buffer);
                fflush(stdout);
            } else {
                break;
            }
        }
    }

    fclose(fp);
    return 0;
}