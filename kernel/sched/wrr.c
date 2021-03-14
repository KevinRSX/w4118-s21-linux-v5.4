/*
 * wrr.c -- w4118 hmwk4 weighted round-robin scheduler
 */

#include <linux/sched/wrr.h>

#include "sched.h"

const struct sched_class sched_wrr_class;
static inline void set_next_task_wrr(struct rq *rq, struct task_struct *p);
static void update_curr_wrr(struct rq *rq);

/* Remaining initializtion issues: see Google doc */

/*
 * Called by sched_init. Although rq lock is NOT held, we'd better not allocate
 * memory in this function
 */

void print_wrr_debug(char *str)
{
	pr_info("[WRR DEBUG] %s", str);
}

void init_wrr_rq(struct wrr_rq *wrr_rq)
{
	wrr_rq->curr = NULL;
	INIT_LIST_HEAD(&wrr_rq->head);
	spin_lock_init(&wrr_rq->wrr_rq_lock);

	wrr_rq->wrr_nr_running = 0;
	wrr_rq->wrr_total_weight = 0;
}

/*
 * TODO (Andreas):
 * Call this when you see fit and update the fields afterward when needed, e.g.
 * on_rq should be 1 after it is enqueued, though I don't know if this is really
 * needed when I copied code from rt.c
 */
void init_wrr_entity(struct sched_wrr_entity *wrr)
{
	INIT_LIST_HEAD(&wrr->run_list);
	wrr->time_slice = WRR_DEFAULT_WEIGHT * WRR_TIMESLICE;
	wrr->weight = WRR_DEFAULT_WEIGHT;
	wrr->on_rq = 0;
	wrr->on_list = 0;
}

static inline struct task_struct *wrr_task_of(struct sched_wrr_entity *wrr_se)
{
	return container_of(wrr_se, struct task_struct, wrr);
}

static inline int on_wrr_rq(struct sched_wrr_entity *wrr_se)
{
	return wrr_se->on_rq;
}

/*
 * Put task to the head or the end of the run list without the overhead of
 * dequeue followed by enqueue.
 */
static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int head)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;
	struct wrr_rq *wrr_rq = &rq->wrr;
	struct list_head *wrr_head = &wrr_rq->head;

	if (!on_wrr_rq(wrr_se))
		return; /* should enqueue if not on wrr_rq */

	if (head)
		list_move(&wrr_se->run_list, wrr_head);
	else
		list_move_tail(&wrr_se->run_list, wrr_head);
}

static inline struct wrr_rq *wrr_rq_of_wrr_se(struct sched_wrr_entity *wrr_se)
{
	struct task_struct *p = wrr_task_of(wrr_se);

	return &task_rq(p)->wrr;
}

static inline struct rq *rq_of_wrr_se(struct sched_wrr_entity *wrr_se)
{
	struct task_struct *p = wrr_task_of(wrr_se);

	return task_rq(p);
}

static inline struct wrr_rq *wrr_rq_of_se(struct sched_wrr_entity *wrr_se)
{
	struct rq *rq = rq_of_wrr_se(wrr_se);

	return &rq->wrr;
}

/* below are class-specific necessary scheduling functions */

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	/* TODO: implement */
	struct sched_wrr_entity *wrr_se = &p->wrr;
	struct wrr_rq *wrr_rq = &rq->wrr;

	WARN_ON_ONCE(wrr_se->on_list);

	list_add_tail(&wrr_se->run_list, &wrr_rq->head);

	wrr_se->on_rq = 1;

	/* Not handling group tasks. So, just +=1 */
	wrr_rq->wrr_nr_running += 1;
	wrr_rq->wrr_total_weight += wrr_se->weight;
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	/* TODO: implement */
	struct sched_wrr_entity *wrr_se = &p->wrr;
	struct wrr_rq *wrr_rq = wrr_rq_of_wrr_se(wrr_se);

	WARN_ON(&rq->wrr != wrr_rq);
	WARN_ON_ONCE(!wrr_se->on_list);

	wrr_se->on_rq = 0;

	list_del(&wrr_se->run_list);

	wrr_rq->wrr_nr_running -= 1;
	wrr_rq->wrr_total_weight -= wrr_se->weight;
}

static void yield_task_wrr(struct rq *rq)
{
	requeue_task_wrr(rq, rq->curr, 0);
}

/*
 * Preempt the current task with a newly woken task if needed:
 */
static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p,
				   int flags)
{
	/* wrr does not preempt current task when a new task arrives */
	/* We may not need but called with existence unchecked. */
}

/* We know rq_lock is held when this is called */
static struct task_struct *
pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	struct task_struct *p;
	struct list_head *head = &rq->wrr.head;
	struct sched_wrr_entity *wrr_se;

	WARN_ON_ONCE(prev || rf);

	if (!sched_wrr_runnable(rq))
		return NULL;

	if (head->next == head)
		return NULL;

	wrr_se = list_entry(head->next, struct sched_wrr_entity, run_list);
	p = wrr_task_of(wrr_se);

	set_next_task_wrr(rq, p);

	return p;
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *p)
{
	update_curr_wrr(rq);
}

static inline void set_next_task_wrr(struct rq *rq, struct task_struct *p)
{
	p->se.exec_start = rq_clock_task(rq);
}


#ifdef CONFIG_SMP
static int balance_wrr(struct rq *rq, struct task_struct *p,
		       struct rq_flags *rf)
{
	/* TODO: implement */
	return -1;
}

/*
 * Compares the weight of wrr_rq on different CPUs and returns the one that
 * has the lowest weight
 */
static int select_task_rq_wrr(struct task_struct *p, int cpu, int sd_flag,
			      int flags)
{
	int i, ret = cpu;
	int weight, min_weight = __INT_MAX__;
	struct rq *rq;

	for_each_online_cpu(i) {
		rq = cpu_rq(i);
		rcu_read_lock();
		weight = rq->wrr.wrr_total_weight;
		if (weight < min_weight) {
			min_weight = weight;
			ret = i;
		}
		rcu_read_unlock();
	}

	return ret;
}

static void rq_online_wrr(struct rq *rq)
{
	/* We may not need this */
}

static void rq_offline_wrr(struct rq *rq)
{
	/* We may not need this */
}

/*
 * If we are not running and we are not going to reschedule soon, we should
 * try to push tasks away now
 */
static void task_woken_wrr(struct rq *rq, struct task_struct *p)
{
	/* We may not need this */
}

static void switched_from_wrr(struct rq *rq, struct task_struct *p)
{
	/* We may not need this */
}
#endif /* CONFIG_SMP */

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;

	update_curr_wrr(rq);

	if (--p->wrr.time_slice)
		return;

	p->wrr.time_slice = p->wrr.weight * WRR_TIMESLICE;

	if (wrr_se->run_list.prev != wrr_se->run_list.next) {
		requeue_task_wrr(rq, p, 0);
		resched_curr(rq);
	}
}

static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *task)
{
	if (!task)
		return 0;
	return task->wrr.weight * WRR_TIMESLICE;
}

static void prio_changed_wrr(struct rq *rq, struct task_struct *p, int oldprio)
{
	/* We may not need but called with existence unchecked. */
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
	/* We may not need but called with existence unchecked. */
}

/*
 * Update the current task's runtime statistics. Skip current tasks that
 * are not in our scheduling class.
 *
 * I don't know why new values are updated to se as opposed to wrr
 * but this is the approach of update_curr_rt. Maybe it is time
 * accounting. However, this is called before being checked in core.c.
 * We have to implement it.
 */
static void update_curr_wrr(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	u64 delta_exec;
	u64 now;

	if (curr->sched_class != &sched_wrr_class)
		return;

	now = rq_clock_task(rq);
	delta_exec = now - curr->se.exec_start;
	if (unlikely((s64)delta_exec <= 0))
		return;

	schedstat_set(curr->se.statistics.exec_max,
		      max(curr->se.statistics.exec_max, delta_exec));

	curr->se.sum_exec_runtime += delta_exec;
	account_group_exec_runtime(curr, delta_exec);

	curr->se.exec_start = now;
	cgroup_account_cputime(curr, delta_exec);

	/* The remaining code in update_curr_rt deal with rt_rq statistics */
}

const struct sched_class sched_wrr_class = {
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
