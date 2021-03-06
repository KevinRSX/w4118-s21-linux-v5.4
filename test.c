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

int main(void)
{
	printf("# of CPUs: %d/%d\n", get_nprocs(), get_nprocs_conf());

	struct wrr_info buf = { MAX_CPUS };

	//Garbage values
	buf.nr_running[3] = 133337;
	buf.total_weight[7] = 133337;

	int ret;

	// TODO: Set some processes to use SCHED_WRR
	// Useful function: sched_setscheduler

	printf("get_wrr_info:\n");
	ret = syscall(__NR_SYSCALL_GET_WRR_INFO, &buf);
	if (ret)
		fprintf(stderr, "err: %s\n", strerror(errno));

	print_wrr_info(buf);

	printf("set_wrr_weight: %d\n", 20);
	ret = syscall(__NR_SYSCALL_SET_WRR_WEIGHT, 20);
	if (ret)
		fprintf(stderr, "err: %s\n", strerror(errno));

	return 0;
}