#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ 0 // file descriptor de lectura
#define WRITE 1 // file descriptor de escritura

int
main(void)
{
	srandom(time(NULL));

	long int r = random();

	int parent_child_fds[2];
	int child_parent_fds[2];

	if (pipe(parent_child_fds) < 0) {
		printf("error in pipe\n");
		exit(-1);
	}

	if (pipe(child_parent_fds) < 0) {
		printf("error in pipe\n");
		exit(-1);
	}

	printf("Hola, PID <%d>\n", getpid());
	printf("- IDs del primer pipe: [%d,%d]\n",
			parent_child_fds[READ], parent_child_fds[WRITE]);
	printf("- IDs del segundo pipe: [%d,%d]\n",
			child_parent_fds[READ], child_parent_fds[WRITE]);

	int res;

	pid_t child_id = fork();

	if (child_id < 0) {
		printf("error in fork\n");
		exit(-1);
	}

	if (child_id == 0) {
		// HIJO
		long int value;

		close(parent_child_fds[WRITE]);
		close(child_parent_fds[READ]);

		res = read(parent_child_fds[READ], &value, sizeof value);

		if (res < 0) {
			printf("error in read - child\n");
			exit(-1);
		}

		printf("Donde fork me devuelve 0:\n");
		printf("- getpid me devuelve: <%d>\n", getpid());
		printf("- getppid me devuelve: <%d>\n", getppid());
		printf("- recibo valor <%ld> via fd=%d\n", value, parent_child_fds[READ]);
		printf("- reenvio valor en fd=%d y termino\n", child_parent_fds[WRITE]);

		res = write(child_parent_fds[WRITE], &value, sizeof value);

		if (res < 0) {
			printf("error in write - child\n");
			exit(-1);
		}

		close(parent_child_fds[READ]);
		close(child_parent_fds[WRITE]);
	} else {
		// PADRE
		long int recv_value;

		close(parent_child_fds[READ]);
		close(child_parent_fds[WRITE]);

		printf("Donde fork me devuelve <%d>:\n", child_id);
		printf("- getpid me devuelve: <%d>\n", getpid());
		printf("- getppid me devuelve: <%d>\n", getppid());
		printf("- valor random: <%ld>\n", r);
		printf("- envio valor <%ld> a través de fd=%d\n",
				r, parent_child_fds[WRITE]);

		res = write(parent_child_fds[WRITE], &r, sizeof r);

		if (res < 0) {
			printf("error in write - parent\n");
			exit(-1);
		}

		res = read(child_parent_fds[READ], &recv_value, sizeof recv_value);

		if (res < 0) {
			printf("error in read - parent\n");
			exit(-1);
		}

		wait(NULL);

		printf("Hola, de nuevo PID <%d>:\n", getpid());
		printf("- recibi valor <%ld> via fd=%d\n",
				recv_value, child_parent_fds[READ]);

		close(child_parent_fds[READ]);
		close(parent_child_fds[WRITE]);
	}

	return 0;
}