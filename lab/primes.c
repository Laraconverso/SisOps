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

<<<<<<< Updated upstream
=======


/////////////////////////
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
	int primo;

	if (read(left_fd, &primo, sizeof(int)) <= 0) {
		close(left_fd);
		exit(0);
	}

	printf("primo %d\n", primo);
    fflush(stdout);

	int fds[2];
	pipe(fds);

	pid_t pid = fork();
	if (pid == 0) {
		close(fds[WRITE]);
		close(left_fd);
		filter_primes(fds[READ]);
		close(fds[READ]);
		exit(0);
	} else {
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

>>>>>>> Stashed changes
int
main(int argc, char *argv[])
{
	if (argc < 2) {
		exit(ERROR);
	}

<<<<<<< Updated upstream
	int last_num = atoi(argv[1]);

	int fds[2];
	if (pipe(fds) == ERROR) {
		perror("Error al crear pipe.");
=======
	int n = atoi(argv[1]);

	int fds[2];
	if (pipe(fds) < 0) {
		perror("Error al crear pipe");
>>>>>>> Stashed changes
		exit(ERROR);
	}

	pid_t pid = fork();
	if (pid < 0) {
<<<<<<< Updated upstream
		perror("Error al crear el fork.");
		exit(ERROR);
	} else if (pid == 0) {
		close(fds[WRITE]);
		filter_primes(fds[READ]);
		exit(0);
	} else {
		close(fds[READ]);
		for (int i = 2; i <= last_num; i++) {
=======
		perror("Error al crear fork");
		exit(ERROR);
	}

	if (pid == 0) {  // Proceso hijo
		close(fds[WRITE]);
		filter_primes(fds[READ]);
		close(fds[READ]);
		exit(0);
	} else {  // Proceso padre
		close(fds[READ]);

		for (int i = 2; i <= n; i++) {
>>>>>>> Stashed changes
			write(fds[WRITE], &i, sizeof(int));
		}

		close(fds[WRITE]);
		wait(NULL);
	}

	exit(0);
<<<<<<< Updated upstream
}
=======
}

>>>>>>> Stashed changes
