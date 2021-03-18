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

#define __NR_SYSCALL_GET_WRR_INFO 436
#define __NR_SYSCALL_SET_WRR_WEIGHT 437

#define SCHED_WRR 7
#define MAX_CHILDREN 1000

struct wrr_info {
	int num_cpus;
	int nr_running[MAX_CPUS];
	int total_weight[MAX_CPUS];
};

void print_wrr_info(struct wrr_info buf, FILE *f)
{
	int txt_col_alm = 15;
	int num_col_alm = 7;
	fprintf(f, "Total # of CPUs: %d\n", buf.num_cpus);

	fprintf(f, "%-*s", txt_col_alm, "CPU No.");
	for (int i = 0; i < buf.num_cpus; i++)
		fprintf(f, "\t%-*d", num_col_alm, i);
	fprintf(f, "\n");

	fprintf(f, "%-*s", txt_col_alm, "nr_running");
	for (int i = 0; i < buf.num_cpus; i++)
		fprintf(f, "\t%-*d", num_col_alm, buf.nr_running[i]);
	fprintf(f, "\n");

	fprintf(f, "%-*s", txt_col_alm, "total_weight");
	for (int i = 0; i < buf.num_cpus; i++)
		fprintf(f, "\t%-*d", num_col_alm, buf.total_weight[i]);
	fprintf(f, "\n");
}

void get_and_print_wrr_info(FILE *f, char *filename)
{
	int ret;
	struct wrr_info buf = { MAX_CPUS };

	printf("wrr_info written to %s\n", filename);
	fprintf(f, "wrr_info:\n====================\n");
	ret = syscall(__NR_SYSCALL_GET_WRR_INFO, &buf);
	if (ret < 0)
		fprintf(stderr, "err: %s\n", strerror(errno));

	print_wrr_info(buf, f);
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

int bear_cpu_child(int weight)
{
	pid_t pid;
	pid = fork();

	if (!pid) {
		set_wrr_weight(weight);
		while (1)
			;
	} else if (pid < 0) {
		fprintf(stderr, "fork err: %s", strerror(errno));
		exit(-1);
	}
	return pid;
}

int bear_io_child(int weight)
{
	pid_t pid;
	pid = fork();

	if (!pid) {
		set_wrr_weight(weight);
		while (1) {
			printf("My pid: %d\n", getpid());
			sleep(1);
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
	int nr_cpu, nr_io; /* no. of children of CPU/IO-bound processes */
	int cpu_pids[MAX_CHILDREN];
	int io_pids[MAX_CHILDREN];
	int other_pids[MAX_CHILDREN];
	int cpu_no, is_mixed, mcnt = 0;
	int iter = 5;
	char filename[25];

	if (argc != 4) {
		fprintf(stderr, "usage: %s nr_cpu nr_io bool_mixed_test\n", argv[0]);
		exit(1);
	}

	cpu_no = get_nprocs();
	//printf("# of CPUs: %d/%d\n", get_nprocs(), get_nprocs_conf());

	nr_cpu = atoi(argv[1]);
	if (nr_cpu <= 0 || nr_cpu >= 1000) {
		fprintf(stderr, "nr_children must be positive and under \
			1000\n");
		exit(1);
	}

	nr_io = atoi(argv[2]);
	if (nr_io <= 0 || nr_io >= 1000) {
		fprintf(stderr, "nr_children must be positive and under \
			1000\n");
		exit(1);
	}

	is_mixed = atoi(argv[3]);
	if (is_mixed != 0 && is_mixed != 1) {
		fprintf(stderr, "bool_mixed_test must be either 1 or 0.\n");
		exit(1);
	}

	for (int i = 0; i < nr_cpu; i++) {
		int weight = i == 0 ? 5 : 1;
		cpu_pids[i] = bear_cpu_child(weight);
		setscheduler(cpu_pids[i], SCHED_WRR);
	}

	for (int i = 0; i < nr_io; i++) {
		int weight = i == 0 ? 20 : 10;
		io_pids[i] = bear_io_child(weight);
		setscheduler(io_pids[i], SCHED_WRR);
	}

	if (is_mixed) {
		for (int i = 0; i < 1; i++) {
			int weight = 1;
			other_pids[mcnt] = bear_cpu_child(weight);
			setscheduler(other_pids[mcnt++], SCHED_OTHER);
			other_pids[mcnt] = bear_cpu_child(weight);
			setscheduler(other_pids[mcnt++], SCHED_IDLE);
			//other_pids[mcnt] = bear_cpu_child(weight);
			//setscheduler(other_pids[mcnt++], SCHED_FIFO);

			weight = 10;
			other_pids[mcnt] = bear_io_child(weight);
			setscheduler(other_pids[mcnt++], SCHED_OTHER);
			other_pids[mcnt] = bear_io_child(weight);
			setscheduler(other_pids[mcnt++], SCHED_IDLE);
			//other_pids[mcnt] = bear_io_child(weight);
			//setscheduler(other_pids[mcnt++], SCHED_FIFO);
		}
	}

	snprintf(filename, 25, "./test_sample_%d_core.txt", cpu_no);
	FILE *f = fopen(filename, "w");
	do {
		get_and_print_wrr_info(f, filename);
		sleep(10);
	} while (iter--);

	for (int i = 0; i < nr_cpu; i++) {
		kill(cpu_pids[i], SIGKILL);
	}

	for (int i = 0; i < nr_io; i++) {
		kill(io_pids[i], SIGKILL);
	}

	if (is_mixed) {
		for (int i = 0; i < mcnt; i++) {
			kill(other_pids[i], SIGKILL);
		}
	}
	fclose(f);
	printf("WRR PROGRAM ENDS HERE. SEE RESULTS IN %s\n", filename);
	return 0;
}
