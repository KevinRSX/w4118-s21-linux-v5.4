/*
 * wrr.c -- w4118 hmwk4 weighted round-robin scheduler
 */

#include <linux/sched/wrr.h>

#include "sched.h"

const struct sched_class sched_wrr_class;
static inline void set_next_task_wrr(struct rq *rq, struct task_struct *p);
static void update_curr_wrr(struct rq *rq);
static void wrr_periodic_balance(void);

#define print_wrr_debug(fmt, ...) \
	pr_info("[WRR DEBUG] " pr_fmt(fmt), ##__VA_ARGS__)

/* Remaining initializtion issues: see Google doc */

/*
 * Called by sched_init. Although rq lock is NOT held, we'd better not allocate
 * memory in this function
 */

void init_wrr_rq(struct wrr_rq *wrr_rq)
{
	wrr_rq->curr = NULL;
	INIT_LIST_HEAD(&wrr_rq->head);
	spin_lock_init(&wrr_rq->wrr_rq_lock);

	wrr_rq->wrr_nr_running = 0;
	wrr_rq->wrr_total_weight = 0;
}

#ifdef CONFIG_SMP

struct hrtimer wrr_balance_timer;
static enum hrtimer_restart sched_wrr_periodic_timer(struct hrtimer *timer)
{
	wrr_periodic_balance();
	hrtimer_forward_now(timer, ns_to_ktime(WRR_BALANCE_PERIOD));
	return HRTIMER_RESTART;
}

void init_wrr_balancer(struct hrtimer *timer)
{
	hrtimer_init(timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_HARD);
	timer->function = sched_wrr_periodic_timer;
	hrtimer_start(timer, ns_to_ktime(WRR_BALANCE_PERIOD),
			HRTIMER_MODE_REL_HARD);
}
#endif /* CONFIG_SMP */

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

static inline struct rq *rq_of_wrr_rq(struct wrr_rq *wrr_rq)
{
	return container_of(wrr_rq, struct rq, wrr);
}

/* below are class-specific necessary scheduling functions */

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;
	struct wrr_rq *wrr_rq;

	wrr_rq = wrr_rq_of_se(wrr_se);

	WARN_ON(on_wrr_rq(wrr_se));
	WARN_ON_ONCE(wrr_se->on_list);

	spin_lock(&wrr_rq->wrr_rq_lock);
	list_add_tail(&wrr_se->run_list, &wrr_rq->head);

	wrr_se->time_slice = wrr_se->weight * WRR_TIMESLICE;
	wrr_se->on_rq = 1;
	wrr_se->on_list = 1;
	wrr_rq->wrr_nr_running += 1;
	wrr_rq->wrr_total_weight += wrr_se->weight;
	spin_unlock(&wrr_rq->wrr_rq_lock);

}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;
	struct wrr_rq *wrr_rq;

	wrr_rq = wrr_rq_of_se(wrr_se);

	WARN_ON(!on_wrr_rq(wrr_se));
	WARN_ON_ONCE(!wrr_se->on_list);

	spin_lock(&wrr_rq->wrr_rq_lock);
	list_del_init(&wrr_se->run_list);

	wrr_rq->wrr_nr_running -= 1;
	wrr_rq->wrr_total_weight -= wrr_se->weight;

	wrr_se->on_rq = 0;
	wrr_se->on_list = 0;
	spin_unlock(&wrr_rq->wrr_rq_lock);
}

static void yield_task_wrr(struct rq *rq)
{
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
	struct wrr_rq *wrr_rq = &rq->wrr;

	if (wrr_rq->wrr_nr_running)
		return 1;

	//wrr_nr_running = 0 at this moment (IDLE)

	return wrr_newidle_balance(rq, rf) != 0;
}

static inline bool is_per_cpu_kthread(struct task_struct *p)
{
	if (!(p->flags & PF_KTHREAD))
		return false;

	if (p->nr_cpus_allowed != 1)
		return false;

	return true;
}

int can_migrate_task(struct task_struct *p, struct rq *src_rq,
		     struct rq *tar_rq)
{
#ifndef CONFIG_SMP
	struct wrr_rq *src_wrr_rq = &src_rq->wrr;
#endif
	if (!p) {
		print_wrr_debug("p is null???");
		return 0;
	}

	/* Task running check */
	if (task_running(src_rq, p)) {
		print_wrr_debug("cant_migrate_task: task_running");
		return 0;
	}
#ifndef CONFIG_SMP
	if (src_wrr_rq->curr == p)
		return 0;
#endif

	if (!cpumask_test_cpu(tar_rq->cpu, p->cpus_ptr))
		return false;

	if (is_per_cpu_kthread(p))
		return cpu_online(tar_rq->cpu);

	return cpu_active(tar_rq->cpu);
}

static void wrr_periodic_balance(void)
{
	int i, weight;
	int min_cpu = 0, max_cpu = 0;
	int min_weight = INT_MAX, max_weight = 0;
	struct task_struct *p;
	struct rq *source_rq, *target_rq, *tmp_rq; /* source->max, target->min */
	struct sched_wrr_entity *wrr_se;
	struct wrr_rq *tmp_wrr_rq, *source_wrr_rq;

	print_wrr_debug("wrr_periodic_balance");
	source_rq = target_rq = NULL;
	p = NULL;
	for_each_online_cpu(i) {
		tmp_rq = cpu_rq(i);

		rcu_read_lock();
		tmp_wrr_rq = &tmp_rq->wrr;
		if (tmp_wrr_rq->wrr_total_weight > max_weight) {
			max_cpu = i;
			max_weight = tmp_wrr_rq->wrr_total_weight;
			source_rq = tmp_rq;
		} else if (tmp_wrr_rq->wrr_total_weight < min_weight) {
			min_cpu = i;
			min_weight = tmp_wrr_rq->wrr_total_weight;
			target_rq = tmp_rq;
		}
		rcu_read_unlock();
	}

	if (unlikely(!source_rq || !target_rq))
		return;

	print_wrr_debug("source cpu %d weight %d target cpu %d, weight %d",
			max_cpu, max_weight, min_cpu, min_weight);

	double_lock_balance(source_rq, target_rq);

	source_wrr_rq = &source_rq->wrr;

	if (source_wrr_rq->wrr_nr_running <= 1)
		goto out;
	
	list_for_each_entry_reverse(wrr_se, &source_wrr_rq->head, run_list) {
		weight = wrr_se->weight;
		if (min_weight + 2 * weight < max_weight) {
			p = wrr_task_of(wrr_se);
			break;
		}
	}

	if (!p || !can_migrate_task(p, source_rq, target_rq))
		goto out;

	// migrate p from source_rq to target_rq
	print_wrr_debug("ready to migrate with weight %d", weight);
out:
	double_unlock_balance(source_rq, target_rq);
	return;
}

int wrr_newidle_balance(struct rq *this_rq, struct rq_flags *rf)
{
	struct wrr_rq *this_wrr_rq = &this_rq->wrr;
	struct sched_wrr_entity *wrr_se;
	struct rq *rq_to_be_pulled;
	struct rq *rq;
	struct wrr_rq *wrr_rq;
	struct task_struct *p;
	int this_cpu = this_rq->cpu;
	int target_cpu = -1;
	int pulled_task = 0;	/*Quite useless*/
	int weight, max_weight = 0;
	int i;

	/* Finding the target CPU*/
	for_each_online_cpu(i) {
		rq = cpu_rq(i);
		wrr_rq = &rq->wrr;
		if (this_wrr_rq == wrr_rq || wrr_rq->wrr_nr_running < 2)
			continue;

		rcu_read_lock();
		weight = wrr_rq->wrr_total_weight;
		if (weight > max_weight) {
			max_weight = weight;
			target_cpu = i;
			rq_to_be_pulled = rq;
		}
		rcu_read_unlock();
	}

	if (target_cpu == -1)
		return pulled_task; /* CPUs are not heavy */

	double_lock_balance(this_rq, rq_to_be_pulled);

	wrr_rq = &rq_to_be_pulled->wrr;

	if (wrr_rq->wrr_nr_running <= 1) {
		// print_wrr_debug("275: CPU is poor.");
		goto out;
	}

	wrr_se = list_last_entry(&wrr_rq->head, struct sched_wrr_entity,
				 run_list);
	p = wrr_task_of(wrr_se);

	if (!can_migrate_task(p, rq_to_be_pulled, this_rq))
		goto out;

	/* Pull to this_rq */
	deactivate_task(rq_to_be_pulled, p, 0);
	set_task_cpu(p, this_cpu);
	activate_task(this_rq, p, 0);

	print_wrr_debug("cpu #%d robbed cpu#%d", this_cpu, target_cpu);
out:
	double_unlock_balance(this_rq, rq_to_be_pulled);
	return pulled_task;
}

/*
 * Compares the weight of wrr_rq on different CPUs and returns the one that
 * has the lowest weight
 */
static int select_task_rq_wrr(struct task_struct *p, int cpu, int sd_flag,
			      int flags)
{
	int i, ret = cpu;
	int weight, min_weight = INT_MAX;
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
	struct sched_wrr_entity *wrr = &p->wrr;

	wrr->time_slice = WRR_DEFAULT_WEIGHT * WRR_TIMESLICE;
	wrr->weight = WRR_DEFAULT_WEIGHT;

	if (task_on_rq_queued(p)) {
		/*
		 * We were most likely switched from sched_rt, so
		 * kick off the schedule if running, otherwise just see
		 * if we can still preempt the current task.
		 */
		if (rq->curr == p)
			resched_curr(rq);
		else
			check_preempt_curr(rq, p, 0);
	}
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

static inline int on_null_domain(struct rq *rq)
{
	return unlikely(!rcu_dereference_sched(rq->sd));
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
