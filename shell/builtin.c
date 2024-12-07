#include "builtin.h"

const char *EXIT = "exit";
const char *CD = "cd";
const char *PWD = "pwd";
const char *HISTORY = "history";

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, EXIT) != 0) {
		return false;
	}

	return true;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	char *cmd_copy = malloc(strlen(cmd) + 1);
	strcpy(cmd_copy, cmd);

	char *right = '\0';
	right = split_line(cmd_copy, ' ');
	if (strcmp(cmd_copy, CD) != 0) {
		free(cmd_copy);
		return false;
	}

	if (right[0] == '\0') {
		char *home = getenv("HOME");
		if (home == NULL || chdir(home) != 0) {
			perror("chdir($HOME) failed");
			free(cmd_copy);
			return true;
		}
	} else {
		if (chdir(right) != 0) {
			perror("chdir() failed");
			free(cmd_copy);
			return true;
		}
	}

	char *arg = getcwd(NULL, 0);
	snprintf(prompt, PRMTLEN, "(%s)", arg);

	free(arg);
	free(cmd_copy);
	return true;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, PWD) != 0) {
		return false;
	}

	char *arg = getcwd(NULL, 0);
	if (arg == NULL) {
		perror("pwd didn't work");
		return false;
	}

	printf("%s\n", arg);

	free(arg);
	return true;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	return false;
}
