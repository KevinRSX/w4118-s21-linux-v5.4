#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define SCHED_WRR 7

#define INIT_PID 1

char policies[20][20];

void init_policies()
{
	strcpy(policies[SCHED_WRR], "SCHED_WRR");
	strcpy(policies[SCHED_RR], "SCHED_RR");
	strcpy(policies[SCHED_FIFO], "SCHED_FIFO");
	strcpy(policies[SCHED_OTHER], "SCHED_OTHER");
	strcpy(policies[SCHED_BATCH], "SCHED_BATCH");
	strcpy(policies[SCHED_IDLE], "SCHED_IDLE");
	strcpy(policies[SCHED_DEADLINE], "SCHED_DEADLINE");
}

void print_info(pid_t pid) /* more info could be added */
{
	int id;

	id = sched_getscheduler(pid);
	printf("%d\t%d\t\t%s\n", pid, id, policies[id]);
}

int main(int argc, char **argv)
{
	int ret;
	struct sched_param param = { 0 };
	pid_t pid;

	if (argc != 2) {
		printf("usage: %s pid\n", argv[0]);
		exit(1);
	}

	pid = atoi(argv[1]);

	init_policies();

	printf("pid\tpolicy_id\tpolicy_name\n");

	print_info(pid);

	return 0;
}
