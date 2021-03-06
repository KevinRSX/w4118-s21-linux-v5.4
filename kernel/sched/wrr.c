/*
 * wrr.c -- w4118 hmwk4 weighted round-robin scheduler
 */

#include "sched.h"

/*
 * TODO:
 *  - define sched_wrr_entity include/linux/sched.h
 *  - define wrr_rq kernel/sched/sched.h
 *  - register wrr_rq to rq, sched_wrr_class to sched_class
 */

const struct sched_class sched_wrr_class = {
	.next			= &rt_sched_class,
	.enqueue_task		= enqueue_task_wrr,
	.dequeue_task		= dequeue_task_wrr,
	.yield_task		= yield_task_wrr,

	.check_preempt_curr	= check_preempt_curr_wrr,

	.pick_next_task		= pick_next_task_wrr,
	.put_prev_task		= put_prev_task_wrr,
	.set_next_task          = set_next_task_wrr,

#ifdef CONFIG_SMP
	.balance		= balance_wrr,
	.select_task_rq		= select_task_rq_wrr,
	.set_cpus_allowed       = set_cpus_allowed_common,
	.rq_online              = rq_online_wrr,
	.rq_offline             = rq_offline_wrr,
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
