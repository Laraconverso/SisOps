#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#ifndef NARGS
#define NARGS 4
#endif

#define ERROR -1
#define END_STRING '\0'
#define NEW_LINE '\n'

void
execute(char *args[])
{
	pid_t child_id = fork();
	if (child_id < 0) {
		perror("Error al crear el fork.");
		exit(ERROR);
	} else if (child_id == 0) {
		execvp(args[0], args);
		perror("Error en execvp");
		exit(ERROR);
	} else {
		wait(NULL);
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		exit(ERROR);
	}

	char *line = NULL;
	size_t size = 0;
	int chars_read;

	char *args[NARGS + 2] = { 0 };
	args[0] = argv[1];

	int cant_args = 1;

	while ((chars_read = getline(&line, &size, stdin)) != ERROR) {
		if (line[chars_read - 1] == NEW_LINE) {
			line[chars_read - 1] = END_STRING;
		}
		
		args[cant_args] = line;
		cant_args++;

		if (cant_args == NARGS + 1) {
			execute(args);
			cant_args = 1;
		}
		free(line);
		line = NULL;
	}

	if (cant_args > 1) {
		args[cant_args] = NULL;
		execute(args);
	}

	free(line);
	return 0;
}
