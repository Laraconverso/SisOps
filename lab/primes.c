#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1
#define ERROR -1

void
filter_primes(int left_fd)
{
	int prime;

	if (read(left_fd, &prime, sizeof(int)) <= 0) {
		close(left_fd);
		exit(0);
	}

	printf("primo %d\n", prime);
	fflush(stdout);

	int fds[2];
	if (pipe(fds) == ERROR) {
		perror("Error al crear pipe");
		exit(ERROR);
	}

	pid_t pid = fork();
	if (pid < 0) {
		perror("Error al crear fork");
		exit(ERROR);
	} else if (pid == 0) {
		close(fds[WRITE]);
		close(left_fd);
		filter_primes(fds[READ]);
		close(fds[READ]);
		exit(0);
	} else {
		close(fds[READ]);

		int num;
		while (read(left_fd, &num, sizeof(int)) > 0) {
			if (num % prime != 0) {
				write(fds[WRITE], &num, sizeof(int));
			}
		}

		close(fds[WRITE]);
		close(left_fd);
		wait(NULL);
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		exit(ERROR);
	}

	int last_num = atoi(argv[1]);

	int fds[2];
	if (pipe(fds) == ERROR) {
		perror("Error al crear pipe.");
		exit(ERROR);
	}

	pid_t pid = fork();
	if (pid < 0) {
		perror("Error al crear el fork.");
		exit(ERROR);
	} else if (pid == 0) {
		close(fds[WRITE]);
		filter_primes(fds[READ]);
		exit(0);
	} else {
		close(fds[READ]);
		for (int i = 2; i <= last_num; i++) {
			write(fds[WRITE], &i, sizeof(int));
		}

		close(fds[WRITE]);
		wait(NULL);
	}

	exit(0);
}