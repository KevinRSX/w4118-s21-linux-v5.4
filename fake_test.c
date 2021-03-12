#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
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

int born_idle_child()
{
	pid_t pid;
	pid = fork();
	if (!pid) {
		while (1)
			;
	} else if (pid < 0) {
		fprintf(stderr, "fork err: %s", strerror(errno));
		return -1;
	}
	printf("> idle child is borned and running.\n");
	return pid;
}

int main()
{
	pid_t pid = 0;
	int ret;
	struct sched_param param = { 0 };

	init_policies();

	pid_t child_pid = born_idle_child();

	printf("pid\tpolicy_id\tpolicy_name\n");

	// Show original info
	pid = child_pid;
	print_info(pid);

	// SCHED_BATCH
	ret = sched_setscheduler(pid, SCHED_BATCH, &param);
	if (ret < 0)
		fprintf(stderr, "err: %s\n", strerror(errno));
	print_info(pid);

	// SCHED_IDLE (Cannot set new policy anymore after set to IDLE)
	ret = sched_setscheduler(pid, SCHED_IDLE, &param);
	if (ret < 0)
		fprintf(stderr, "err: %s\n", strerror(errno));
	print_info(pid);

	// SCHED_IDLE (Cannot set new policy anymore after set to IDLE)
	ret = sched_setscheduler(pid, SCHED_OTHER, &param);
	if (ret < 0)
		fprintf(stderr, "err: %s\n", strerror(errno));
	print_info(pid);

	/*
	param.sched_priority = 99;
	ret = sched_setscheduler(pid, SCHED_FIFO, &param);
	if (ret < 0)
		fprintf(stderr, "err: %s\n", strerror(errno));
	print_info(pid);

	param.sched_priority = 99;
	ret = sched_setscheduler(pid, SCHED_RR, &param);
	if (ret < 0)
		fprintf(stderr, "err: %s\n", strerror(errno));
	print_info(pid);

	pid = 1; // init
	print_info(pid);
	*/

	kill(child_pid, SIGKILL);
	printf("> idle child is killed.\n");
	return 0;
}
