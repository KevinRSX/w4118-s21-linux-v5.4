#define _GNU_SOURCE
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>
#include <stdbool.h>
#include <sched.h>
#include <sys/sysinfo.h>

#define MAX_CPUS 8
#define STR_LEN 50

#define __NR_SYSCALL_GET_WRR_INFO 436
#define __NR_SYSCALL_SET_WRR_WEIGHT 437

#define SCHED_WRR 7
#define NR_CHILDREN 5

struct wrr_info {
	int num_cpus;
	int nr_running[MAX_CPUS];
	int total_weight[MAX_CPUS];
};

/* wrapper of the syscall*/
void set_wrr_weight(int weight)
{
	int ret;
	printf("set_wrr_weight: %d\n", weight);
	ret = syscall(__NR_SYSCALL_SET_WRR_WEIGHT, weight);
	if (ret)
		fprintf(stderr, "err: %s\n", strerror(errno));
}

int counter;

/*wrapper of the sched_setscheduler*/
void setscheduler(pid_t pid, int policy)
{
	/* The param is for Non-RT policies */
	struct sched_param non_rt_param = { 0 };
	struct sched_param rt_param = { 1 };

	int ret;
	if (policy == SCHED_RR || policy == SCHED_FIFO) {
		ret = sched_setscheduler(pid, policy, &rt_param);
	} else {
		ret = sched_setscheduler(pid, policy, &non_rt_param);
	}

	if (ret < 0) {
		fprintf(stderr, "err: %s\n", strerror(errno));
		exit(ret);
	}
}

void handle_sigterm(int sig)
{
	printf("%d\n", counter);
}

pid_t bear_cpu_child(int weight)
{
	pid_t pid;
	pid = fork();
	int t = 0;

	if (!pid) {
		set_wrr_weight(weight);
		signal(SIGTERM, handle_sigterm);
		while (1) {
			t++;
			if (t % 10000  == 0) {
				counter++;
			}
			if (t == 50000) {
				t = 0;
			}
		}
	} else if (pid < 0) {
		fprintf(stderr, "fork err: %s", strerror(errno));
		exit(-1);
	}
	return pid;
}


pid_t bear_weight1_child()
{
	pid_t pid;
	pid = fork();
	int t = 0;

	if (!pid) {
		set_wrr_weight(1);
		while (1)
			;
	} else if (pid < 0) {
		fprintf(stderr, "fork err: %s", strerror(errno));
		exit(-1);
	}
	return pid;
}

int main(int argc, char **argv)
{
	int weight;
	pid_t pid_normal_child, pid_weight1_child;

	if (argc != 2) {
		fprintf(stderr, "usage: %s child_weight", argv[0]);
		exit(1);
	}

	weight = atoi(argv[1]);
	pid_normal_child = bear_cpu_child(weight);
	pid_weight1_child = bear_weight1_child();

	sleep(15);
	kill(pid_normal_child, SIGTERM);
	sleep(1);
	kill(pid_normal_child, SIGKILL);
	kill(pid_weight1_child, SIGKILL);

}
