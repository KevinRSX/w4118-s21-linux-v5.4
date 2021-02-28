#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>
#include <stdbool.h>

#define MAX_CPUS 8

#define __NR_SYSCALL_GET_WRR_INFO 436
#define __NR_SYSCALL_SET_WRR_WEIGHT 437

struct wrr_info {
	int num_cpus;
	int nr_running[MAX_CPUS];
	int total_weight[MAX_CPUS];
};

int main(void)
{
	struct wrr_info *buf;
	int ret;

	buf = malloc(sizeof(struct wrr_info) * 2);
	if (!buf)
		return -1;

	printf("get_wrr_info:\n");
	ret = syscall(__NR_SYSCALL_GET_WRR_INFO, 2);
	if (ret)
		fprintf(stderr, "err: %s\n", strerror(errno));

	printf("set_wrr_weight: %d\n", 20);
	ret = syscall(__NR_SYSCALL_SET_WRR_WEIGHT, 20);
	if (ret)
		fprintf(stderr, "err: %s\n", strerror(errno));

	return 0;
}