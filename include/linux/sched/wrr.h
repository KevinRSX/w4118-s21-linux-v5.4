#ifndef _LINUX_SCHED_WRR_H
#define _LINUX_SCHED_WRR_H

#include <linux/sched.h>

#define MAX_CPUS 8

struct wrr_info {
	int num_cpus;
	int nr_running[MAX_CPUS];
	int total_weight[MAX_CPUS];
};

/*
 * copied and modified rt.h
 * default timeslice is 10 msecs (used only for SCHED_WRR tasks).
 * Timeslices get refilled after they expire.
 */
#define WRR_TIMESLICE		(10 * HZ / 1000)
#define WRR_DEFAULT_WEIGHT	1

#endif /* LINUX_SCHED_WRR_H_ */
