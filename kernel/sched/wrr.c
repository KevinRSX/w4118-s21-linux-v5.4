/*
 * wrr.c -- w4118 hmwk4 weighted round-robin scheduler
 */

#include "sched.h"

/*
 * TODO: Add initialization code (policy, class, entity, etc) at fork time
 */

/*
 * Called by sched_init. While rq lock is NOT held, we'd better not allocate
 * memory in this function
 */
void init_wrr_rq(struct wrr_rq *wrr_rq)
{
	wrr_rq->wrr_nr_running = 0;
	wrr_rq->curr = NULL;
	INIT_LIST_HEAD(&wrr_rq->head);
	spin_lock_init(&wrr_rq->wrr_rq_lock);
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
}

static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int head)
{
}

static void yield_task_wrr(struct rq *rq)
{
	requeue_task_wrr(rq, rq->curr, 0);
}

static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p,
				   int flags)
{
}
static struct task_struct *
pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	struct task_struct *t;
	return t;
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *p)
{
}

static inline void set_next_task_wrr(struct rq *rq, struct task_struct *p)
{
}

static int balance_wrr(struct rq *rq, struct task_struct *p,
		       struct rq_flags *rf)
{
	return -1;
}

static int select_task_rq_wrr(struct task_struct *p, int cpu, int sd_flag,
			      int flags)
{
	return -1;
}

static void rq_online_wrr(struct rq *rq)
{
}

static void rq_offline_wrr(struct rq *rq)
{
}

static void task_woken_wrr(struct rq *rq, struct task_struct *p)
{
}

static void switched_from_wrr(struct rq *rq, struct task_struct *p)
{
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
}

static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *task)
{
	return -1;
}

static void prio_changed_wrr(struct rq *rq, struct task_struct *p, int oldprio)
{
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
}

static void update_curr_wrr(struct rq *rq)
{
}

const struct sched_class wrr_sched_class = {
	.next			= &fair_sched_class,
	.enqueue_task		= enqueue_task_wrr,
	.dequeue_task		= dequeue_task_wrr,
	.yield_task		= yield_task_wrr,

	.check_preempt_curr	= check_preempt_curr_wrr,

	.pick_next_task		= pick_next_task_wrr,
	.put_prev_task		= put_prev_task_wrr,
	.set_next_task		= set_next_task_wrr,

#ifdef CONFIG_SMP
	.balance		= balance_wrr,
	.select_task_rq		= select_task_rq_wrr,
	.set_cpus_allowed       = set_cpus_allowed_common,
	.rq_online		= rq_online_wrr,
	.rq_offline		= rq_offline_wrr,
	.task_woken		= task_woken_wrr,
	.switched_from		= switched_from_wrr,
#endif

	.task_tick		= task_tick_wrr,

	.get_rr_interval	= get_rr_interval_wrr,

	.prio_changed		= prio_changed_wrr,
	.switched_to		= switched_to_wrr,

	.update_curr		= update_curr_wrr,

#ifdef CONFIG_UCLAMP_TASK
	.uclamp_enabled		= 1,
#endif
};
