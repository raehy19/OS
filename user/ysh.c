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

#define NULL 0

typedef enum e_node_type {
	LOGICAL,
	COMMAND,
	REDIRECT,
}t_node_type;

typedef enum e_logical_type {
	NaL,
	ROOT,
	PIPE,
}t_logical_type;

typedef enum e_token_type {
	T_STRING,
	T_PIPE,
	T_SEMICOLON,
	T_AMPERSAND,
	T_REDIRECTING_OUTPUT,
}t_token_type;

typedef struct s_node t_node;
typedef struct s_token_node t_token;

typedef struct s_token_node {
	t_token_type type;
	char *str;
	t_token *next;
}t_token;

typedef struct s_node {
	// Node info
	t_node_type type;
	t_node *left;
	t_node *right;

	// Data to use if Node is logical node
	t_logical_type logical_type;

	// Data to use if Node is command node
	char *command_path;
	char **command_arg;
	int is_background;

	// Data to use if Node is redirect node
	char *redirect_filename;
	int in_fd;
	int out_fd;
	int pid;
}t_node;

t_token *new_token(t_token_type type, char *str) {
	t_token *new;

	new = (t_token *)malloc(sizeof(t_token));
	if (!new)
		return (NULL);
	new->type = type;
	new->str = str;
	new->next = NULL;
	return (new);
}

t_token *lst_last_token(t_token *lst) {
	t_token *temp;

	if (!lst)
		return (NULL);
	temp = lst;
	while (temp->next)
		temp = temp->next;
	return (temp);
}

void token_push(t_token **lst, t_token *new) {
	t_token *temp;

	if (!(*(lst))) {
		*lst = new;
		return;
	}
	temp = lst_last_token(*(lst));
	if (!temp)
		return;
	temp->next = new;
}

int is_string_char(char c) {
	if (!c || c == ' ' || c == '&' || c == '|' || c == '>')
		return (0);
	return (1);
}

char *ft_strndup(const char *s1, int len) {
	char *dst;

	dst = (char *)malloc(len + 1);
	if (!dst)
		return (NULL);
	*(dst + len) = '\0';
	return (memmove(dst, s1, len));
}

t_token *tokenize(char *input) {
	t_token *token_list = NULL;
	int idx = -1;

	// read until end of input cmd, make tokenized list
	while (*(input + ++idx)) {
		// ignore whitespace
		if (*(input + idx) == ' ') {
			while (*(input + idx + 1) == ' ')
				++idx;
		}
			// tokenize PIPE
		else if (*(input + idx) == '|') {
			token_push(&token_list, new_token(T_PIPE, NULL));
		}
			// tokenize redirecting output
		else if (*(input + idx) == '>') {
			token_push(&token_list, new_token(T_REDIRECTING_OUTPUT, NULL));
		}
			// tokenize redirecting output
		else if (*(input + idx) == '&') {
			token_push(&token_list, new_token(T_AMPERSAND, NULL));
		}
			// tokenize semicolon
		else if (*(input + idx) == ';') {
			token_push(&token_list, new_token(T_SEMICOLON, NULL));
		}
			// tokenize string
		else {
			int i = 0;
			while (is_string_char(*(input + idx + i)))
				++i;
			token_push(&token_list, new_token(T_STRING, ft_strndup(input + idx, i)));
			idx += (i - 1);
		}
	}

	return (token_list);
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

		t_token *token_list = tokenize(cmd);

		// print token
		while (token_list) {
			printf("|||%s|||%d\n", token_list->str, token_list->type);
			token_list = token_list->next;
		}

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
