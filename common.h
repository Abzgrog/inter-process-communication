#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_BUFFER 152       
#define SHM_KEY_1  1234      
#define SHM_KEY_2  5678      
#define SEM_KEY_1  1111      
#define SEM_KEY_2  2222      

#define FILE_P1 "p1.txt"
#define FILE_P2 "p2.txt"
#define FILE_OUT "serv2.txt"

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

typedef struct {
    char text[MAX_BUFFER];
} shared_data_t;

#endif // COMMON_H