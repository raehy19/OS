#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define buf_size    128     // Max length of user input
#define max_args    16      // Max number of arguments

int runcmd(char *cmd);      // Run a command.

// Read a shell input.
char *readcmd(char *buf) {
	// Read an input from stdin.
	fprintf(1, "$ ");
	memset(buf, 0, buf_size);
	char *cmd = gets(buf, buf_size);

	// Chop off the trailing '\n'.
	if (cmd) {
		cmd[strlen(cmd) - 1] = 0;
	}

	return cmd;
}

int main(int argc, char **argv) {
	int fd = 0;
	char *cmd = 0;
	char buf[buf_size];

	// Ensure three file descriptors are open.
	while ((fd = open("console", O_RDWR)) >= 0) {
		if (fd >= 3) {
			close(fd);
			break;
		}
	}

	fprintf(1, "EEE3535 Operating Systems: starting ysh\n");

	// Read and run input commands.
	while ((cmd = readcmd(buf)) && runcmd(cmd));

	fprintf(1, "EEE3535 Operating Systems: closing ysh\n");
	exit(0);
}


// Run a command.
int runcmd(char *cmd) {
	if (!*cmd) {
		return 1;
	}                     // Empty command

	// Skip leading white space(s).
	while (*cmd == ' ') {
		cmd++;
	}
	// Remove trailing white space(s).
	for (char *c = cmd + strlen(cmd) - 1; *c == ' '; c--) {
		*c = 0;
	}

	// exit command
	if (!strcmp(cmd, "exit")) {
		return 0;
	}
		// cd command
	else if (!strncmp(cmd, "cd ", 3)) {          // cd command
		if (chdir(cmd + 3) < 0) {
			fprintf(2, "Cannot cd %s\n", cmd + 3);
		}
	} else {
		// EEE3535 Operating Systems
		// Assignment 1: Shell

		// Create a child process
		int pid = fork();

//		fprintf(1, "%d\n", pid);

		// Child process (pid == 0)
		if (pid == 0) {
//			fprintf(1, "child process (pid=%d)\n", getpid());
			char *argv[max_args];

			argv[0] = cmd;
			argv[1] = 0;

			exec(*argv, argv);

			// Print error if exec function fail
			fprintf(2, "Cannot execute %s\n", cmd);
			return (1);
		}
			// Parent process (pid > 0)
		else if (pid > 0) {
			// The parent waits for a child process to finish.
			// wait(NULL) is equivalent to waitpid(/*pid*/ -1, /*status*/ NULL, /*option*/ 0);
			wait(0);
//			fprintf(1, "parent process (pid=%d) of child process (pid=%d)\n", getpid(), pid);
		}
			// fork failed (pid < 0)
		else {
			// Print error if fork function fail
			fprintf(2, "fork() failed\n");
			return (1);
		}
	}
	return 1;
}
