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
	ret = syscall(__NR_SYSCALL_SET_WRR_WEIGHT, weight);
	if (ret)
		fprintf(stderr, "err: %s\n", strerror(errno));
}

int bear_cpu_child()
{
	pid_t pid;
	pid = fork();

	if (!pid) {
		set_wrr_weight(20);
		while (1)
			;
	} else if (pid < 0) {
		fprintf(stderr, "fork err: %s", strerror(errno));
		exit(-1);
	}
	return pid;
}

int main(void)
{
	pid_t pid;

	get_and_print_wrr_info();
	set_wrr_weight(20);
	get_and_print_wrr_info();
	return 0;
}
