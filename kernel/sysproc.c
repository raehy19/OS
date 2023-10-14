#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void) {
	int n;
	argint(0, &n);
	exit(n);
	return 0;  // not reached
}

uint64
sys_getpid(void) {
	return myproc()->pid;
}

uint64
sys_fork(void) {
	return fork();
}

uint64
sys_wait(void) {
	uint64 p;
	argaddr(0, &p);
	return wait(p);
}

uint64
sys_sbrk(void) {
	uint64 addr;
	int n;

	argint(0, &n);
	addr = myproc()->sz;
	if (growproc(n) < 0)
		return -1;
	return addr;
}

uint64
sys_sleep(void) {
	int n;
	uint ticks0;

	argint(0, &n);
	acquire(&tickslock);
	ticks0 = ticks;
	while (ticks - ticks0 < n) {
		if (killed(myproc())) {
			release(&tickslock);
			return -1;
		}
		sleep(&ticks, &tickslock);
	}
	release(&tickslock);
	return 0;
}

uint64
sys_kill(void) {
	int pid;

	argint(0, &pid);
	return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void) {
	uint xticks;

	acquire(&tickslock);
	xticks = ticks;
	release(&tickslock);
	return xticks;
}

//////////   Assignment 2 : System Call and Process   //////////

// A xv6-riscv syscall can take up to six arguments.
#define max_args 6

typedef enum e_state_type {
	Sleep = 1,
	Runnable = 2,
	eXecuting = 4,
	Zombie = 8
}t_state_type;

typedef struct s_proc_info {
	int pid;
	enum procstate state;
	int ppid;
	int time_created;
	char *name;
}t_proc_info;

//////////    //////////   //////////   //////////    //////////

uint64
sys_pstate() {
	// EEE3535 Operating Systems
	// Assignment 2: System Call and Process

	//////////   Assignment 2 : System Call and Process   //////////

	unsigned char options = 0;
	int arg;

	// parse argument
	for (int i = 0; i < max_args; ++i) {
		// read argument value
		argint(i, &arg);

		// arg == S, R, X, Z : set options
		if (arg < 0) {
			options |= (-arg);
		}
			// arg == positive int : => PID add PID to Display List
		else if (arg > 0) {
			printf("pid input : %d\n", arg);
		}
			// (arg == 0) : end of arg
		else
			break;
	}

	printf("options : %d\n", options);

	// 1st element of all proc list
	struct proc *p_ptr = myproc()->all_proc_list;

	// new list and idx for store required data
	t_proc_info p_info_list[NPROC];
	int p_info_list_idx = 0;

	// copy required data from proc list to p_info_list
	for (int i = 0; i < NPROC; ++i, ++p_ptr) {
		// held lock
		acquire(&p_ptr->lock);

		// Disregard Unused state proc
		if (p_ptr->state == UNUSED) {
			// release lock
			release(&p_ptr->lock);
			continue;
		}

		// Copy required data to list
		p_info_list[p_info_list_idx].pid = p_ptr->pid;
		p_info_list[p_info_list_idx].state = p_ptr->state;

		// release lock
		release(&p_ptr->lock);

		// Copy required data to list
		p_info_list[p_info_list_idx].ppid = p_ptr->parent_pid;
		p_info_list[p_info_list_idx].name = p_ptr->name;
		p_info_list[p_info_list_idx].time_created = p_ptr->time_created;

		// increase idx
		++p_info_list_idx;
	}

	printf("PID\tPPID\tState\tRuntime\tName\t\n");

	// print all proc info
	for (int i = 0; i < p_info_list_idx; ++i) {
		printf("%d\t%d\t%d\t%d\t%s\n", p_info_list[i].pid, p_info_list[i].ppid,
			   p_info_list[i].state, p_info_list[i].time_created, p_info_list[i].name);
	}

	return (0);

	//////////    //////////   //////////   //////////    //////////
}
