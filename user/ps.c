#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// A xv6-riscv syscall can take up to six arguments.
#define max_args 6

// Print a help message.
void print_help(int argc, char **argv) {
	fprintf(2, "%s <options: pid or S/R/X/Z>%s\n",
			argv[0], argc > 7 ? ": too many args" : "");
}

//////////   Assignment 2 : System Call and Process   //////////

typedef enum e_arg_type {
	Sleep = -1,
	Runnable = -2,
	eXecuting = -4,
	Zombie = -8
}t_arg_type;

// arg check function cause atoi convert until non-number character
int is_valid_num(char *str) {
	// check if only number exist in arg
	while (*str) {
		if (*str < '0' || *str > '9')
			return (0);
		++str;
	}
	return (1);
}

//////////    //////////   //////////   //////////    //////////

int main(int argc, char **argv) {
	// Print a help message.
	if (argc > 7) {
		print_help(argc, argv);
		exit(1);
	}

	// Argument vector
	int args[max_args];
	memset(args, 0, max_args * sizeof(int));

	/* Assignment 2: System Call and Process
	   Convert the char inputs of argv[] to integers in args[].
	   In this skeleton code, args[] is initialized to zeros,
	   so technically no arguments are passed to the pstate() syscall. */

	//////////   Assignment 2 : System Call and Process   //////////

	// parse arg
	for (int i = 1; i < argc; ++i) {

		// convert arg to int
		int int_val = atoi(argv[i]);

		// if it can convert to int and valid, store and pass to pstate call
		if (int_val && is_valid_num(argv[i])) {
			args[i - 1] = int_val;
		}
			// check arg is option
		else if ((argv[i][0] == 'S' || argv[i][0] == 'R' || argv[i][0] == 'X' ||
				  argv[i][0] == 'Z') && argv[i][1] == '\0') {
			// set args as negative number for each option
			switch (argv[i][0]) {
				case 'S' :
					args[i - 1] = Sleep;
					break;
				case 'R' :
					args[i - 1] = Runnable;
					break;
				case 'X' :
					args[i - 1] = eXecuting;
					break;
				case 'Z' :
					args[i - 1] = Zombie;
					break;
			}
		}
			// print help for invalid arg
		else {
			print_help(argc, argv);
			exit(1);
		}
	}

	//////////    //////////   //////////   //////////    //////////

	// Call the pstate() syscall.
	int ret = pstate(args[0], args[1], args[2], args[3], args[4], args[5]);
	if (ret) {
		fprintf(2, "pstate failed\n");
		exit(1);
	}

	exit(0);
}