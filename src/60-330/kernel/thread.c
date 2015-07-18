/*
 * GeekOS - kernel threads
 *
 * Copyright (C) 2001-2008, David H. Hovemeyer <david.hovemeyer@gmail.com>
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *   
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *  
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdbool.h>
#include <geekos/string.h>
#include <geekos/thread.h>
#include <geekos/kassert.h>
#include <geekos/int.h>
#include <geekos/mem.h>
#include <geekos/workqueue.h>

//ADDED: includes
#include <geekos/timer.h>
#include <60-330/scheduler.h>
#include <60-330/job.h>
#include <60-330/process.h>

int g_num_threads;

static struct thread_queue s_runqueue;

//ADDED: s_newqueue for newly allocated threads to wait in until they are set to
//ready by the long_term_scheduler
static struct thread_queue s_newqueue;

//ADDED: gen_pid()
int gen_pid(void) {
	static int pid = 0;
	//We track the total number of threads to ensure that total kernel threads
	//does not exceed MAX_THREADS
	g_num_threads = pid + 1;
	return pid++;
}

//ADDED: thread_queue_insert_at_priority()
void thread_queue_insert_at_priority(struct thread_queue* list, struct thread* node) {
	//clear the new node's pointers
	node->thread_queue_next = node->thread_queue_prev = 0;
	//insert on an empty list
	if (thread_queue_is_empty(list)) {
		list->head = list->tail = node;
	}
	//insert at the tail if the node has the lowest priority
	//(larger number's indicate lower priority)
	else if (node->process->priority >= list->tail->process->priority) {
		node->thread_queue_prev = list->tail;
		list->tail->thread_queue_next = node;
		list->tail = node;
	}
	//insert at the head if the node has the greatest priority
	//(larger number's indicate lower priority)
	else if (node->process->priority < list->head->process->priority) {
		node->thread_queue_next = list->head;
		list->head->thread_queue_prev = node;
		list->head = node;
	}
	//Otherwise, search the list starting from the tail and insert mid-list
	else {
		struct thread* temp = list->tail;
		while ((node->process->priority < temp->process->priority)
			&& (temp->thread_queue_prev != 0))
				temp = temp->thread_queue_prev;
		node->thread_queue_prev = temp;
		node->thread_queue_next = temp->thread_queue_next;
		temp->thread_queue_next->thread_queue_prev = node;
		temp->thread_queue_next = node;
	}
}

/*
//if priority scheduling is enabled, age the lowest-priority process
//to prevent starvation
void aging() {
	if (increment_runqueue_tail_age() > AGING_QUANTUM) 
	{
		escalate_runqueue_tail_priority();
	}
}

//ADDED: age the lowest priority thread
int increment_runqueue_tail_age(void) {
	return ++(s_runqueue.tail->age);
}

//ADDED: advance the lowest priority thread to a higher priority
void escalate_runqueue_tail_priority(void) {
	bool iflag = int_begin_atomic();
	s_runqueue.tail->age = 0;
	s_runqueue.tail->process->priority = (s_runqueue.tail->thread_queue_prev->process->priority) - 1;
	thread_make_runnable(thread_queue_remove_last(&s_runqueue));
	int_end_atomic(iflag);
}

//ADDED: remove for aging
IMPLEMENT_LIST_REMOVE_LAST(thread_queue, thread)
*/

/*-----------------------------------------------------------------------
 * Implementation
 *----------------------------------------------------------------------- */
 
IMPLEMENT_LIST_CLEAR(thread_queue, thread)
IMPLEMENT_LIST_IS_EMPTY(thread_queue, thread)
IMPLEMENT_LIST_APPEND(thread_queue, thread)
IMPLEMENT_LIST_REMOVE_FIRST(thread_queue, thread)

/*
 * Idle thread; ensures that at least one thread is
 * always running or runnable.
 */
static void thread_idle(ulong_t arg)
{
	while (true) {
		thread_yield();
	}
	/* does not return */
}

/*#define DEBUG_RUNQUEUE*/

#ifdef DEBUG_RUNQUEUE
static void thread_dump_runnable(void)
{
	struct thread *thread;
	cons_printf("runqueue:");
	for (thread = thread_queue_get_first(&s_runqueue);
	     thread != 0;
	     thread = thread_queue_next(thread)) {
		cons_printf(" [%p]", thread);
	}
	cons_printf("\n");
}
#endif

/*
 * Workqueue callback function to free resources used by
 * a thread that has exited or been killed.
 */
static void thread_destroy(void *thread_)
{
	struct thread *thread = thread_;

	KASSERT(thread->state == THREAD_EXITED || thread->state == THREAD_KILLED);

	/* cons_printf("destroying thread %d\n", thread->process->pid);*/

	/* TODO: user space teardown */
	//ADDED: process deallocation
	mem_free(thread->process);
	
	mem_free_frame(mem_pa_to_frame(thread->stack));
	mem_free(thread);
}

/*
 * Detach a reference to a thread.
 * Each thread detaches from itself when it exits.
 * A parent thread detaches from the child when it joins.
 * When the thread's refcount reaches 0, it is scheduled
 * for destruction by the workqueue thread.
 */
static void thread_detach(struct thread *thread)
{
	KASSERT(!int_enabled());
	KASSERT(thread->refcount > 0);
	thread->refcount--;
	if (thread->refcount == 0) {
		cons_printf("PID %d deallocated by work queue\n", thread->process->pid);
		workqueue_schedule_work(&thread_destroy, thread);
	}
}

/*-----------------------------------------------------------------------
 * Interface
 *----------------------------------------------------------------------- */

struct thread *g_current;
volatile int g_need_reschedule;
volatile int g_preemption;

/*
 * Bootstrap main thread, initialize scheduler.
 */
void thread_init(void)
{
	struct thread *main_thread;

	KASSERT(g_current == 0);
	KASSERT(g_need_reschedule == 0);
	KASSERT(g_preemption == false);
	KASSERT(thread_queue_is_empty(&s_runqueue));
	KASSERT(THREAD_CONTEXT_SIZE == sizeof(struct thread_context));
	KASSERT(THREAD_STACK_PTR_OFFSET == OFFSETOF(struct thread, stack_ptr));

	/* bootstrap main thread */
	main_thread = (struct thread *) mem_alloc(sizeof(struct thread));
	memset(main_thread, '\0', sizeof(struct thread));
	main_thread->stack = (void *) KERN_STACK;
	main_thread->state = THREAD_RUNNING;
	main_thread->refcount = 1;
	g_current = main_thread;
	
	main_thread->process = mem_alloc(sizeof(struct process));	//ADDED: process allocation
	g_scheduler = FCFS;											//ADDED: initialize default scheduler
	main_thread->process->pid = gen_pid();						//ADDED: set process ID of main thread (0)
	main_thread->process->priority = DEFAULT_PRIORITY;			//ADDED: set a default priority for the main thread
	main_thread->process->time.arrival = 1;						//ADDED: initialize arrival time
	//main_thread->age = 0;										//ADDED: initialize age to 0
	
	/* create idle thread */
	//Idle thread; ensures that at least one thread is always running or runnable.
	thread_create(thread_idle, 0UL, THREAD_DETACHED);
}

/*
 * Create and start a new kernel-only thread.
 * Returns a pointer to the new kernel thread, or 0 if
 * there is not enough memory to create the new thread.
 */
 //NOTE: thread create is used only by geekos' processes
struct thread *thread_create(thread_func_t *start_func, ulong_t arg, thread_mode_t mode)
{
	struct thread *thread;
	void *stack;

	thread = mem_alloc(sizeof(struct thread));
	stack = mem_frame_to_pa(mem_alloc_frame(FRAME_KSTACK, 0));

	/* initialize the thread */
	memset(thread, '\0', sizeof(struct thread));
	thread->stack = stack;
	thread->refcount = 1; /* each thread has an implicit self-reference */
	if (mode == THREAD_ATTACHED) {
		/* parent (current thread) holds a reference */
		thread->parent = g_current;
		thread->refcount++;
	}

	//ADDED: initialize the thread state to NEW and set arrival time
	thread->state = THREAD_NEW;	
	//ADDED: process allocation
	thread->process = mem_alloc(sizeof(struct process));
	
	thread->process->time.arrival = (g_numticks > 0) ? g_numticks : 1;	//prevent 0 arrival time
	
	/* start it running */
	thread_bootstrap(thread, start_func, arg);
	KASSERT(thread->stack_ptr != 0);
	thread_make_runnable(thread);
	

	thread->process->pid = gen_pid();				//ADDED: initialize process ID
	thread->process->priority = DEFAULT_PRIORITY;	//ADDED: initialize with a default priority
	//thread->age = 0;						//ADDED: initialize age to 0
	
	return thread;
}

//ADDED: thread_create_from_job() for use by long_term_scheduler()
/*
 * Create and start a new kernel-only thread.
 * Returns a pointer to the new kernel thread, or 0 if
 * there is not enough memory to create the new thread.
 */
struct thread *thread_create_from_job(const struct job* new_job, ulong_t arg, thread_mode_t mode)
{
	struct thread *thread;
	void *stack;

	thread = mem_alloc(sizeof(struct thread));
	stack = mem_frame_to_pa(mem_alloc_frame(FRAME_KSTACK, 0));

	/* initialize the thread */
	memset(thread, '\0', sizeof(struct thread));
	thread->stack = stack;
	thread->refcount = 1; /* each thread has an implicit self-reference */
	if (mode == THREAD_ATTACHED) {
		/* parent (current thread) holds a reference */
		thread->parent = g_current;
		thread->refcount++;
	}

	
	//ADDED: process allocation
	thread->process = mem_alloc(sizeof(struct process));
	
	thread->process->pid = gen_pid();	//ADDED: initialize pid
	//thread->age = 0;					//ADDED: initialize age to 0
	
	//ADDED: set priority or estimated burst time if applicable; otherwise, default
	if (g_scheduler == SJF)
		thread->process->priority = new_job->cpu_burst;
	else if (g_scheduler == PRIORITY)
		thread->process->priority = new_job->priority;
	else
		thread->process->priority = DEFAULT_PRIORITY;
	
	/* start it running */
	thread_bootstrap(thread, new_job->fn_ptr, arg);
	KASSERT(thread->stack_ptr != 0);
	thread_make_new(thread);
	
	return thread;
}

//ADDED: Add a given thread to the newqueue.
void thread_make_new(struct thread *thread)
{
	bool iflag = int_begin_atomic();
	cons_printf("Making new thread: PID: %d, Priority: %d\n", thread->process->pid, thread->process->priority);
	thread->state = THREAD_NEW;
	thread->process->time.arrival = g_numticks;
	SCHEDULER[g_scheduler](&s_newqueue, thread);
	int_end_atomic(iflag);
}

//ADDED: Merge the newqueue with the runqueue
void merge_new_with_runnable(void)
{
	struct thread* thread;
	bool iflag = int_begin_atomic();
	while (! thread_queue_is_empty(&s_newqueue)) {
		thread = thread_queue_remove_first(&s_newqueue);
		narrator(thread->process->pid, "ready");
		thread->state = THREAD_READY;
		SCHEDULER[g_scheduler](&s_runqueue, thread);
	}
	KASSERT(thread_queue_is_empty(&s_newqueue));
	thread_queue_clear(&s_newqueue);
	int_end_atomic(iflag);
}

//ADDED: wait for a list of attached (child) processes to terminate
void wait_for_children(struct thread** thread, int num_thread, struct simulation_stat * stat) {
	int i;
	bool iflag;
	for (i=0; i < num_thread; ++i) {
		//reschedule the parent to allow the child time to execute
		thread_yield();
		
		KASSERT(g_current == thread[i]->parent);
		iflag = int_begin_atomic();
		//wait until child has exited or been killed
		thread_wait_until(&thread[i]->waitqueue, &thread_not_running, thread[i]);
		//calculate child's runtime statistics
		stat[i] = calculate_simulation_stat(thread[i]->process->time, thread[i]->process->pid);
		//remove the parent's reference, schedule child for destruction
		thread_detach(thread[i]);
		int_end_atomic(iflag);
	}
}

//ADDED: for testing
#ifdef DEBUG_NEWQUEUE
	void traverse_runqueue(void) {
		struct thread_queue * list = &s_runqueue;
		KASSERT(!thread_queue_is_empty(list));
		struct thread* thread = list->tail;
		cons_printf("traversing... pid %d priority %d (tail)\n", thread->process->pid, thread->process->priority);
		thread = thread->thread_queue_prev;
		while (thread != list->head) {
			cons_printf("traversing... pid %d priority %d \n", thread->process->pid, thread->process->priority);
			thread = thread->thread_queue_prev;
		}
		cons_printf("traversing... pid %d priority %d (head)\n", thread->process->pid, thread->process->priority);
	}
	void reverse_traverse_runqueue(void) {
		struct thread_queue * list = &s_runqueue;
		KASSERT(!thread_queue_is_empty(list));
		struct thread* thread = list->head;
		cons_printf("traversing... pid %d priority %d (head)\n", thread->process->pid, thread->process->priority);
		thread = thread->thread_queue_next;
		while (thread != list->tail) {
			cons_printf("traversing... pid %d priority %d \n", thread->process->pid, thread->process->priority);
			thread = thread->thread_queue_next;
		}
		cons_printf("traversing... pid %d priority %d (tail)\n", thread->process->pid, thread->process->priority);
	}
#endif

/*
 * Thread end of life function.
 * Any thread waiting for this thread to exit is notified.
 * When this thread's refcount reaches 0, it releases its
 * resources and picks a new thread to run.
 */
void thread_exit(int exitcode)
{
	struct thread *thread = g_current;

	/* make sure ints are disabled */
	if (int_enabled()) {
		int_disable();
	}

	/* decrease thread's refcount */
	thread_detach(thread);
	thread->exitcode = exitcode;
	thread->state = THREAD_EXITED;

	//ADDED: set termination time
	thread->process->time.termination = g_numticks;
	
	//ADDED: during a simulation, print when a thread exits
	narrator(thread->process->pid, "terminated");
	
	/* if there is a parent, notify it that the child has exited */
	if (thread->refcount > 0) {
		thread_wakeup(&thread->waitqueue);
	}

	/* pick a new thread to run */
	thread_schedule();

	/* Convince gcc that this is a noreturn function. */
	while (true);
}

/*
 * Wait for a child thread to exit.
 * After the function returns, the parent should not make any further
 * references to the child thread object.
 */
int thread_join(struct thread *child)
{
	bool iflag;
	int exitcode;

	KASSERT(g_current == child->parent);

	iflag = int_begin_atomic();

	/* wait until child has exited or been killed */
	thread_wait_until(&child->waitqueue, &thread_not_running, child);

	/* get exit code (TODO: killed status?) */
	exitcode = child->exitcode;

	/* remove the parent's reference, schedule child for destruction */
	thread_detach(child);

	int_end_atomic(iflag);

	return exitcode;
}

/*
 * Atomically add current thread to given thread queue
 * and choose another thread to run.
 */
void thread_wait(struct thread_queue *queue)
{
	KASSERT(!int_enabled());
	
	//ADDED: during a simulation, print when a thread exits
	narrator(g_current->process->pid, "waiting");
	
	thread_relinquish_cpu();
	thread_queue_append(queue, g_current);
	thread_schedule();
}

/*
 * Park current thread in given thread queue.
 * Interrupts must be enabled, but preemption must be disabled.
 */
void thread_park(struct thread_queue *queue)
{
	KASSERT(!g_preemption);

	int_disable();
	g_preemption = true;
	thread_wait(queue);
	g_preemption = false;
	int_enable();
}

/*
 * Wake up all threads waiting in given thread queue.
 */
void thread_wakeup(struct thread_queue *queue)
{
	/* remove all threads from queue, make them runnable */
	while (!thread_queue_is_empty(queue))
                thread_wakeup_one(queue);
}

/*
 * Wake up one thread waiting in given thread queue.
 */
void thread_wakeup_one(struct thread_queue *queue)
{
	KASSERT(!int_enabled());
	struct thread *thread = thread_queue_remove_first(queue);
	if (thread) {
		/*cons_printf("waking up thread %p\n", thread);*/
		thread_make_runnable(thread);
	}
}

/*
 * Wait until given thread predicate returns true.
 */
void thread_wait_until(struct thread_queue *queue, bool (*pred)(struct thread *), struct thread *thread)
{
	KASSERT(!int_enabled());
	while (!pred(thread)) {
		thread_wait(queue);
	}
}

/*
 * Predicate function that returns true when a thread's refcount reaches 0.
 */
bool thread_refcount_is_zero(struct thread *thread)
{
	KASSERT(thread->refcount >= 0);
	return thread->refcount == 0;
}

/*
 * Predicate function that returns true when an attached child
 * thread is no longer running.
 */
bool thread_not_running(struct thread *thread)
{
	KASSERT(thread->refcount > 0);
	KASSERT(thread->parent == g_current);
	return thread->state == THREAD_EXITED || thread->state == THREAD_KILLED;
}

/*
 * Yield the CPU to another runnable thread.
 */
void thread_yield(void)
{
	bool iflag = int_begin_atomic();
	thread_relinquish_cpu();
	thread_make_runnable(g_current);
	thread_schedule();
	int_end_atomic(iflag);
}

/*
 * Called to indicate that the current thread is giving up the CPU.
 */
void thread_relinquish_cpu(void)
{
	struct thread *thread = g_current;
	KASSERT(thread->state == THREAD_RUNNING);

	/* FIXME: sample num_ticks */
	thread->num_ticks = 0;
}

/*
 * Thread scheduler: find the next thread to run.
 */
struct thread *thread_next_runnable(void)
{
	struct thread *next;
	KASSERT(!int_enabled());
#ifdef DEBUG_RUNQUEUE
	thread_dump_runnable();
#endif
	next = thread_queue_remove_first(&s_runqueue);
	KASSERT(next);
	next->state = THREAD_RUNNING;
	//ADDED: during a simulation, print when a thread exits
	narrator(next->process->pid, "running");
	return next;
}

/*
 * Add given thread to the runqueue.
 */
void thread_make_runnable(struct thread *thread)
{
	bool iflag = int_begin_atomic();
	thread->state = THREAD_READY;	
	//ADDED: during a simulation, print when a thread exits
	narrator(thread->process->pid, "ready");
	//ADDED: use scheduler of choice to order enqueue threads appropriately
	SCHEDULER[g_scheduler](&s_runqueue, thread);
	int_end_atomic(iflag);
}

/*
 * Schedule a runnable thread.
 * Assumes that the current thread has been placed on an appropriate
 * thread queue (if it is going to be run again).
 */
void thread_schedule(void)
{
	KASSERT(!int_enabled());
	thread_switch_to(thread_next_runnable());
}
