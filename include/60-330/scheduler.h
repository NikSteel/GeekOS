#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <geekos/thread.h>
#include <60-330/job.h>

#define RR_QUANTUM1 8
#define RR_QUANTUM2 32
#define RR_QUANTUM3 128

//#define AGING_QUANTUM 25

typedef enum {
	FCFS, SJF, PRIORITY, RR
} scheduler_t;

extern volatile scheduler_t g_scheduler;	   /* specifies the index of the scheduling function to use */
extern void (* const SCHEDULER[4])(struct thread_queue*, struct thread*);

void long_term_scheduler(struct thread** const, const struct job*, int, thread_mode_t);
void FCFS_scheduler(struct thread_queue*, struct thread*);
void SJF_scheduler(struct thread_queue*, struct thread*);
void PRIORITY_scheduler(struct thread_queue*, struct thread*);
void RR_scheduler(void);

#endif