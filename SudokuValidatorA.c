#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <sys/wait.h>

#define SIZE 81

int sudoku[9][9];
int result;

int checkRows() {
    int i, j, res;
    int valid = 0;
    for (int i = 0; i < 9; i++) {
        res = 0;
        for (int j = 0; j < 9; j++) {
            int valueMatrix = sudoku[i][j];
            if (valueMatrix < 1 || valueMatrix > 9) 
                valid = -1;
            res += valueMatrix;
        }
        if (result != 45) {
            valid = -1;
        }
    }
    return valid;
}

int checkColumns() {
    int i, j, res;
    int valid = 0;
    for (i = 0; i < 9; i++) {
        result = 0;
        long threadId = syscall(SYS_gettid);
        printf("En la revision de columnas el siguiente es un thread en ejecucion: %ld\n", threadId);
        int checked[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; 
        for (j = 0; j < 9; j++) {
            int valueMatrix = sudoku[j][i];
            if (valueMatrix < 1 || valueMatrix > 9) 
                valid = -1;
            res += valueMatrix;
        }
        if (result != 45) {
            valid = -1;
        }
    }

    return valid;
}

int checkSubarray() {
    for (int i = 0; i < 9; i += 3) {
        for (int j = 0; j < 9; j += 3) {
            int checked[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; 
            for (int k = i; k < i + 3; k++) {
                for (int m = j; m < j + 3; m++) {
                    int value = sudoku[k][m];
                    if (value < 1 || value > 9) {
                        return -1;
                    }
                    checked[value - 1] = 1;
                }
            }
            int result = 0;
            for (int j = 0; j < 9; j++) {
                result += checked[j];
            }
            if (result != 9) {
                return -1;
            }
        }
    }
    return 0;
}

void* columnCheck(void* arg) {
    result = checkColumns();

    pthread_exit(0);

}

int main(int argc, char* argv[]) {
    char* path = argv[1];
    int fileDesc = open(path, O_RDONLY);

    char* f;
    f = (char *) mmap(0, SIZE, PROT_READ, MAP_PRIVATE, fileDesc, 0);
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            sudoku[i][j] = f[i*9 + j] - '0';
        }
    }

    int subArrayResult = checkSubarray();

    pid_t pid;
    pid = getpid();
    printf("%d\n", pid);

    pthread_t columnThread;
    int forkId = fork();
    char pidString[10];
    sprintf(pidString, "%d", pid);

    if (forkId  == 0) {
        printf("Estado de padre y threads. El padre tiene un id: %s\n", pidString);
        execlp("ps", "ps", "-p", pidString, "-lLf", NULL);
    }
    else {
        if (pthread_create(&columnThread, NULL, columnCheck, NULL) == 0)
            printf("Thread para chequear columnas creado.\n");
        
        if (pthread_join(columnThread, NULL) == 0)
            printf("El thread para chequear columnas ha concluido.\n");

        wait(NULL);

        int rowResult = checkRows();
        if (subArrayResult && result && rowResult) {
            printf("La solucion del sudoku es incorrecta.\n");
        }
        else {
            printf("Sudoku resulto correctamente.\n");
        }


        if (fork() == 0) {
            printf("Antes de terminar el estado de este proceso y sus threads es:\n");
            execlp("ps", "ps", "-p", pidString, "-lLf", NULL);
        }
        else {
            wait(NULL);
            return 0;
        }

    }

    return 0;
}