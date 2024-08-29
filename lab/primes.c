#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1
#define ERROR -1


void filter_primes(int left_fd) {
    int primo;

    if (read(left_fd, &primo, sizeof(int)) <= 0) {
        close(left_fd);
        exit(0);
    }

    printf("primo %d\n", primo);

    int fds[2];
    pipe(fds);

    pid_t pid = fork();
    if (pid == 0) { 
		// Proceso hijo
        close(fds[WRITE]);
        close(left_fd);
        filter_primes(fds[READ]);
        close(fds[READ]);
        exit(0);
    } else { 
		// Proceso padre
        close(fds[READ]);

        int num;
        while (read(left_fd, &num, sizeof(int)) > 0) {
            if (num % primo != 0) {
                write(fds[WRITE], &num, sizeof(int));
            }
        }

        close(fds[WRITE]);
        close(left_fd);
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        exit(ERROR);
    }

    int n = atoi(argv[1]);

    int fds[2];
	if (pipe(fds) < 0) {
        perror("Error al crear pipe");
        exit(ERROR);
    }

    pid_t pid = fork();
	if (pid < 0) {
        perror("Error al crear fork");
        exit(ERROR);
    }

    if (pid == 0) { // Proceso hijo
        close(fds[WRITE]);
        filter_primes(fds[READ]);
        close(fds[READ]);
        exit(0);
    } else { // Proceso padre
        close(fds[READ]);

        for (int i = 2; i <= n; i++) {
            write(fds[WRITE], &i, sizeof(int));
        }

        close(fds[WRITE]);
        wait(NULL);
    }

    return 0;
}
