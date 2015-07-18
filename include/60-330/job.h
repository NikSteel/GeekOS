#ifndef JOBS_H
#define JOBS_H

#include <geekos/mem.h>

#define JOB_DELAY 10

struct job {
	void (*fn_ptr) (ulong_t);
	int cpu_burst;
	int priority;
};

//run at startup to initialize mutex
void s_job_mutex_init(void);

//jobs to be scheduled
void fn1(ulong_t);
void fn2(ulong_t);
void fn3(ulong_t);
void interrupt_toggler(ulong_t);
void peterson_player(ulong_t);
void mutex_poet(ulong_t);

#endif