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
#define MAX_CHILDREN 1000

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

int counter[2] = { 0 };

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

int bear_cpu_child(int weight, int index)
{
	pid_t pid;
	pid = fork();
	int t = 0;

	if (!pid) {
		set_wrr_weight(weight);
		while (1) {
			t++;
			if (t % 2000 == 0) {
				counter[index]++;
			}
			if (t == 10000) {
				//printf("pid: %d, weight: %d, counter: %d\n",
				//       getpid(), weight, counter);
				t = 0;
			}
		}
	} else if (pid < 0) {
		fprintf(stderr, "fork err: %s", strerror(errno));
		exit(-1);
	}
	return pid;
}

int main(int argc, char **argv)
{
	int nr_cpu, iter = 1;
	int *weight;
	int cpu_pids[MAX_CHILDREN];
	//counter = (int *) malloc(2 * sizeof(int));
	//counter[0] = counter[1] = 0;


	if (argc != 2) {
		fprintf(stderr, "usage: %s nr_cpu\n", argv[0]);
		exit(1);
	}

	nr_cpu = atoi(argv[1]);
	if (nr_cpu <= 0 || nr_cpu >= 1000) {
		fprintf(stderr, "nr_children must be positive and under \
			1000\n");
		exit(1);
	}

	weight = (int*) malloc(nr_cpu * sizeof(int));
	for (int i = 0; i < nr_cpu; i++) {
		scanf("%d", weight + i);
		counter[i] = 0;
	}

	for (int i = 0; i < nr_cpu; i++) {
		cpu_pids[i] = bear_cpu_child(weight[i], i);
		setscheduler(cpu_pids[i], SCHED_WRR);
	}

	do {
		/*
		for (int i = 0; i < nr_cpu; i++) {
			printf("process %d: weight %d, counter %d\n", cpu_pids[i], weight[i], counter[i]);
		}
		*/
		sleep(5);
	} while (iter--);

	printf("weight %d: counter %d\n weight %d: counter %d\n", weight[0],
	       counter[0], weight[1], counter[1]);

	for (int i = 0; i < nr_cpu; i++) {
		kill(cpu_pids[i], SIGKILL);
	}
	return 0;
}
