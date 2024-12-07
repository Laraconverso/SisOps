#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		char key[BUFLEN], value[BUFLEN];
		int index = block_contains(eargv[i], '=');

		if (index >= 0) {
			get_environ_key(eargv[i], key);
			get_environ_value(eargv[i], value, index);
			setenv(key,
			       value,
			       1);  // el 1 es para que sobrescriba si la variable ya esiste
		} else {
			perror("Error: argumento/s invalido/s");
			exit(-1);
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd;
	if (flags & O_CREAT) {  // si flags incluye el O_CREAT, se debe crear el
		                // archivo si no existe => llamo a la funcion
		                // open con permisos de write and read
		fd = open(file, flags, S_IWUSR | S_IRUSR);
	} else {
		fd = open(file, flags);
	}

	if (fd < 0) {
		perror("Error al abrir el archivo");
		exit(-1);
	}

	return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC: {
		// spawns a command
		e = (struct execcmd *) cmd;  // casteo a execcmd
		set_environ_vars(e->eargv, e->eargc);

		int result = execvp(e->argv[0], e->argv);
		if (result < 0) {
			perror("ERROR: fallo execvp!");
			free_command(cmd);
			exit(-1);
		}


		break;
	}

	case BACK: {
		// runs a command in background
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);

		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		int fd;
		r = (struct execcmd *) cmd;
		if (strlen(r->out_file) > 0) {
			fd = open_redir_fd(r->out_file,
			                   O_CLOEXEC | O_CREAT |  // crear archivo
			                           O_WRONLY |     // escribir
			                           O_TRUNC);      // truncar
			dup2(fd, STDOUT_FILENO);  // salida estandar
		}

		if (strlen(r->in_file) > 0) {
			fd = open_redir_fd(r->in_file,
			                   O_CLOEXEC | O_RDONLY);  // solo lectura
			dup2(fd, STDIN_FILENO);  // entrada estandar
		}

		if (strlen(r->err_file) > 0) {
			// comparo, y si es &1 (redirecion del stderr al stdout)
			// redirecciono a la fuerza
			if (strcmp(r->err_file, "&1") == 0) {
				dup2(STDOUT_FILENO, STDERR_FILENO);
			} else {
				fd = open_redir_fd(r->err_file,
				                   O_CLOEXEC | O_CREAT | O_RDWR |  // aca uso el de lectura y escritura
				                           O_TRUNC);
				dup2(fd, STDERR_FILENO);
			}
		}

		cmd->type = EXEC;
		exec_cmd(cmd);
		break;
	}

	case PIPE: {
		// pipes two commands
		p = (struct execcmd *) cmd;
		int fd[2];

		if (pipe(fd) < 0) {
			perror("Error al crear el Pipe. \n");
			exit(-1);
		}

		int f_l = fork();
		if (f_l < 0) {
			perror("Error al crear el fork izq. \n");
			close(fd[READ]);
			close(fd[WRITE]);
			exit(-1);
		} else if (f_l == 0) {
			/// HIJO IZQ
			setpgid(0, 0);
			close(fd[READ]);
			// output -> fd entrada
			dup2(fd[WRITE], STDOUT_FILENO);
			close(fd[WRITE]);
			exec_cmd(p->leftcmd);
			exit(-1);
		} else {
			// PADRE

			int f_d = fork();
			if (f_d < 0) {
				perror("Error al crear el fork der. \n");
				close(fd[READ]);
				close(fd[WRITE]);
				exit(-1);
			} else if (f_d == 0) {
				setpgid(0, 0);
				close(fd[WRITE]);
				// output -> fd entrada
				dup2(fd[READ], STDIN_FILENO);
				close(fd[READ]);
				exec_cmd(p->rightcmd);
				exit(-1);
			} else {
				// PADRE
				setpgid(0, 0);
				close(fd[READ]);
				close(fd[WRITE]);
				// ESPERO HIJOS
				waitpid(f_l, NULL, 0);
				waitpid(f_d, NULL, 0);
				// free the memory allocated
				// for the pipe tree structure
				free_command(cmd);
				exit(0);
			}
		}

		break;
	}
	}
}