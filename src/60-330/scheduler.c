#include <geekos/thread.h>
#include <geekos/timer.h>
#include <geekos/int.h>
#include <60-330/scheduler.h>
#include <60-330/job.h>

//global thread scheduler index
volatile scheduler_t g_scheduler;

//a constant array of pointers to scheduler functions:
void (* const SCHEDULER[4])(struct thread_queue*, struct thread*) = {
	&FCFS_scheduler, &SJF_scheduler, &PRIORITY_scheduler, &FCFS_scheduler };
//NOTE: SCHEDULER[3] == FCFS_scheduler() because RR will sort using FCFS

//similar to windows or mac osx, our long term scheduler simply loads
//all new jobs into memory 
void long_term_scheduler(struct thread ** const threads, const struct job* joblist, int num_job, thread_mode_t mode) {
	int i;
	
	//create new kernel threads for each job, save a record of the threads that were created
	for (i = 0; i < num_job; ++i)
		threads[i] = thread_create_from_job(&joblist[i], 0, mode);
		
	//move the newly created threads from the new queue to the ready queue
	//in a single atomic (all jobs have the same arrival time)
	merge_new_with_runnable();
}

//schedule a process by adding it to the end of the READY queue
void FCFS_scheduler(struct thread_queue* runqueue, struct thread* thread) {
	thread_queue_append(runqueue, thread);
}

//schedule a process by inserting it into the READY queue at 
//a position corresponding to the thread's estimated burst time
//which was stored in the thread's priority field
void SJF_scheduler(struct thread_queue* runqueue, struct thread* thread) {
	thread_queue_insert_at_priority(runqueue, thread);
}

//schedule a process by inserting it into the READY queue at 
//a position corresponding to the thread's priority
void PRIORITY_scheduler(struct thread_queue* runqueue, struct thread* thread) {
	thread_queue_insert_at_priority(runqueue, thread);
}

//reschedule a process when its time slice expires
void RR_scheduler() {
	if (g_current->num_ticks > g_quantum)
		g_need_reschedule = 1;
}
