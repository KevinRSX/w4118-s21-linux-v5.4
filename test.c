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

#define __NR_SYSCALL_GET_WRR_INFO 436
#define __NR_SYSCALL_SET_WRR_WEIGHT 437

#define SCHED_WRR 7
#define MAX_CHILDREN 1000

struct wrr_info {
	int num_cpus;
	int nr_running[MAX_CPUS];
	int total_weight[MAX_CPUS];
};

void print_wrr_info(struct wrr_info buf)
{
	int txt_col_alm = 15;
	int num_col_alm = 7;
	printf("Total # of CPUs: %d\n", buf.num_cpus);

	printf("%-*s", txt_col_alm, "CPU No.");
	for (int i = 0; i < buf.num_cpus; i++)
		printf("\t%-*d", num_col_alm, i);
	printf("\n");

	printf("%-*s", txt_col_alm, "nr_running");
	for (int i = 0; i < buf.num_cpus; i++)
		printf("\t%-*d", num_col_alm, buf.nr_running[i]);
	printf("\n");

	printf("%-*s", txt_col_alm, "total_weight");
	for (int i = 0; i < buf.num_cpus; i++)
		printf("\t%-*d", num_col_alm, buf.total_weight[i]);
	printf("\n");
}

void get_and_print_wrr_info()
{
	int ret;
	struct wrr_info buf = { MAX_CPUS };

	printf("get_wrr_info:\n");
	ret = syscall(__NR_SYSCALL_GET_WRR_INFO, &buf);
	if (!ret)
		fprintf(stderr, "err: %s\n", strerror(errno));

	print_wrr_info(buf);
}

/* wrapper of the syscall*/
void set_wrr_weight(int weight)
{
	int ret;
	printf("set_wrr_weight: %d\n", weight);
	ret = syscall(__NR_SYSCALL_SET_WRR_WEIGHT, weight);
	if (ret)
		fprintf(stderr, "err: %s\n", strerror(errno));
}

int bear_cpu_child()
{
	pid_t pid;
	pid = fork();

	if (!pid) {
		while (1)
			;
	} else if (pid < 0) {
		fprintf(stderr, "fork err: %s", strerror(errno));
		exit(-1);
	}
	return pid;
}

int bear_io_child()
{
	pid_t pid;
	pid = fork();

	if (!pid) {
		while (1) {
			printf("My pid: %d\n", getpid());
		}
	} else if (pid < 0) {
		fprintf(stderr, "fork err: %s", strerror(errno));
		exit(-1);
	}
	return pid;
}

/*wrapper of the sched_setscheduler*/
void setscheduler(pid_t pid, int policy)
{
	/* The param is for Non-RT policies */
	struct sched_param param = { 0 };

	int ret;
	ret = sched_setscheduler(pid, policy, &param);
	if (ret < 0) {
		fprintf(stderr, "err: %s\n", strerror(errno));
		exit(ret);
	}
}

int main(int argc, char **argv)
{
	int nr_children;
	int (*bear_func)();
	int children_pids[MAX_CHILDREN]; 

	if (argc != 3) {
		fprintf(stderr, "usage: %s [c/i] nr_children\n", argv[0]);
		exit(1);
	}
	// printf("# of CPUs: %d/%d\n", get_nprocs(), get_nprocs_conf());

	nr_children = atoi(argv[2]);
	if (nr_children < 0 || nr_children >= 1000) {
		fprintf(stderr, "nr_children must be positive and under 1000\n");
		exit(1);
	}

	if (argv[1][0] == 'c')
		bear_func = &bear_cpu_child;
	else if (argv[1][0] == 'i')
		bear_func = &bear_io_child;
	else {
		fprintf(stderr, "usage: %s [c/i] nr_children\n", argv[0]);
		exit(1);
	}

	printf("Before bearing child:\n");
	get_and_print_wrr_info();

	for (int i = 0; i < nr_children; i++) {
		children_pids[i] = (*bear_func)();
		setscheduler(children_pids[i], SCHED_WRR);
	}

	printf("After bearing child:\n");
	get_and_print_wrr_info();

	return 0;
}
