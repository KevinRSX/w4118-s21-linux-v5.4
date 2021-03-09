#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#define SCHED_WRR 7

int main()
{
	pid_t pid = 0;
	int ret;
	struct sched_param param = { 0 };

	ret = sched_setscheduler(pid, SCHED_OTHER, &param);
	if (ret < 0)
		fprintf(stderr, "err: %s\n", strerror(errno));

	printf("scheduler set to %d\n", sched_getscheduler(pid));

	param.sched_priority = 99;
	ret = sched_setscheduler(pid, SCHED_FIFO, &param);
	if (ret < 0)
		fprintf(stderr, "err: %s\n", strerror(errno));
	printf("scheduler set to %d\n", sched_getscheduler(pid));

	param.sched_priority = 99;
	ret = sched_setscheduler(pid, SCHED_RR, &param);
	if (ret < 0)
		fprintf(stderr, "err: %s\n", strerror(errno));
	printf("scheduler set to %d\n", sched_getscheduler(pid));

	return 0;
}
