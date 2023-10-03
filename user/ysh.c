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
#define STDIN_FILENO 0
#define STDOUT_FILENO 1

// Logical type of node
typedef enum e_logical_type {
	ROOT,
	PIPE,
}t_logical_type;

// Type of token
typedef enum e_token_type {
	T_SEMICOLON,
	T_PIPE,
	T_AMPERSAND,
	T_REDIRECTING_OUTPUT,
	T_STRING,
}t_token_type;

// token struct
typedef struct s_token t_token;
typedef struct s_token {
	// Type of token
	t_token_type type;
	char *str;
	t_token *next;
}t_token;

// node struct
typedef struct s_node t_node;
typedef struct s_node {
	// Pointer of the next node
	t_node *next;

	// Logical type of node : Root(new or command after semicolon) or Pipe(command after pipe)
	t_logical_type logical_type;

	// Command argv
	char **command_arg;

	// If command need to execute background => 1 / else => 0
	int is_background;

	// If there is redirect, redirect filename and fds
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
	if (!c || c == ' ' || c == '&' || c == '|' || c == '>' || c == ';')
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
			// tokenize pipe
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

t_node *new_node(t_logical_type logical_type) {
	t_node *new;

	new = (t_node *)malloc(sizeof(t_node));
	if (!new)
		return (NULL);
	new->next = NULL;
	new->logical_type = logical_type;
	new->command_arg = NULL;
	new->is_background = 0;
	new->redirect_filename = NULL;
	new->in_fd = STDIN_FILENO;
	new->out_fd = STDOUT_FILENO;
	new->pid = 0;
	return (new);
}

t_token *token_shift(t_token **token_list) {
	t_token *temp;

	temp = *token_list;
	if (temp)
		*token_list = (*token_list)->next;
	return (temp);
}

void parse_token_list(t_node **head, t_token **token_list);

void parse_token_list(t_node **head, t_token **token_list) {
	t_token *temp;
	char *cmd_arg[max_args] = {0,};
	int cmd_cnt = -1;

	// Get token by shifting from token list
	temp = token_shift(token_list);

	// Parse Token to Node
	while (temp && temp->type != T_PIPE && temp->type != T_SEMICOLON) {
		// Parse redirecting output
		if (temp->type == T_REDIRECTING_OUTPUT && temp->next && temp->next->type == T_STRING) {
			free(temp);
			temp = token_shift(token_list);
			(*head)->redirect_filename = temp->str;
			free(temp);
		}
			// Parse background execute
		else if (temp->type == T_AMPERSAND && (!temp->next || temp->next->type == T_SEMICOLON)) {
			(*head)->is_background = 1;
			free(temp);
		}
			// Parse command
		else {
			if (++cmd_cnt < max_args)
				cmd_arg[cmd_cnt] = temp->str;
			free(temp);
		}

		// Shift to the next token
		temp = token_shift(token_list);
	}

	// Make command argument
	(*head)->command_arg = malloc(sizeof(char *) * (cmd_cnt + 1));
	if (cmd_cnt == max_args)
		--cmd_cnt;
	for (int i = 0; i < cmd_cnt + 1; ++i) {
		(*head)->command_arg[i] = cmd_arg[i];
	}
	(*head)->command_arg[cmd_cnt + 1] = NULL;

	// If there are token left in token_list, add new node to node_list
	// Then, continue parsing token, by call this function recursively
	if (temp) {
		(*head)->next = new_node((t_logical_type)temp->type);
		free(temp);
		parse_token_list(&((*head)->next), token_list);
	}
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
		t_node *node_list = new_node(ROOT);
		parse_token_list(&node_list, &token_list);

		t_node *temp = node_list;
		while (temp) {
			int fd[2];
			int has_pipe = (temp->next && temp->next->logical_type == PIPE);

			// If it has pipe, create pipe
			if (has_pipe) {
				pipe(fd);
//				printf("pipe created fd[0] : %d | fd[1] : %d\n", fd[0], fd[1]);
			}

			// Create a child process
			int pid = fork();

			// Child process (pid == 0)
			if (pid == 0) {
				fprintf(1, "child process (pid=%d)\n", getpid());

				// If it has redirect, redirect output
				if (temp->redirect_filename) {
					close(STDOUT_FILENO);
					open(temp->redirect_filename, O_WRONLY | O_CREATE | O_TRUNC);
				}

				// If it has pipe
				if (has_pipe) {
					close(STDOUT_FILENO);
					dup(fd[1]);
					close(fd[0]);
					close(fd[1]);
				}

				// Execute command
				exec(temp->command_arg[0], temp->command_arg);

				// Print error if exec function fail
				fprintf(2, "Cannot execute %s\n", temp->command_arg[0]);
				exit(1);
			}
				// Parent process (pid > 0)
			else if (pid > 0) {
				// The parent waits for a child process to finish.
				// wait(NULL) is equivalent to waitpid(/*pid*/ -1, /*status*/ NULL, /*option*/ 0);


				// If it needs to execute background, do not wait for child process to finish
				if (!temp->is_background) {
					wait(NULL);
				}

				// If it has pipe
				if (has_pipe) {
					close(STDIN_FILENO);
					dup(fd[0]);
					close(fd[0]);
					close(fd[1]);
				}
				fprintf(2, "parent process (pid=%d) of child process (pid=%d)\n", getpid(), pid);
			}
				// fork failed (pid < 0)
			else {
				// Print error if fork function fail
				fprintf(2, "fork() failed\n");
				return (1);
			}
			// Execute next node
			temp = temp->next;
		}

//		// print node
//		while (node_list) {
//			printf("logical type : %d | cmdarg1 : %s | cmdarg2 : %s | isbackground : %d | redirect_filename : %s |\n",
//				   node_list->logical_type, node_list->command_arg[0], node_list->command_arg[1],
//				   node_list->is_background, node_list->redirect_filename);
//
//
//			node_list = node_list->next;
//		}

	}
	return 1;
}
