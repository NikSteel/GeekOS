#include <stdbool.h>
#include <geekos/mem.h>
#include <geekos/types.h>
#include <geekos/cons.h>
#include <geekos/timer.h>
#include <geekos/thread.h>
#include <geekos/int.h>
#include <geekos/synch.h>
#include <60-330/job.h>
#include <60-330/stat.h>
#include <60-330/tictactoe.h>
#include <60-330/peterson.h>
#include <60-330/poetry.h>

//delays for pre-emption toggle demo
#define BIG_NUM 500000000LU
#define BIG_INTERVAL 100000000LU

//shared data for Peterson's solution demo
static struct peterson_t s_peterson;

//shared data for mutex lock demo
static struct mutex s_job_mutex;

//run at start up to initialize simulation mutex
void s_job_mutex_init(void) {
	mutex_init(&s_job_mutex);
}

void fn1(ulong_t arg){
	first_response("fn1");
	busy_wait(JOB_DELAY);
	cons_printf("PID %d (%d): Fn1: Running.\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d (%d): Fn1: Running..\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d (%d): Fn1: Running...\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);
	thread_exit(42);
}

void fn2(ulong_t arg){
	first_response("fn2");
	busy_wait(JOB_DELAY);	
	cons_printf("PID %d (%d): Fn2: Running.\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);	
	cons_printf("PID %d (%d): Fn2: Running..\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);	
	cons_printf("PID %d (%d): Fn2: Running...\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);	
	cons_printf("PID %d (%d): Fn2: Running....\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);	
	cons_printf("PID %d (%d): Fn2: Running.....\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);	
	cons_printf("PID %d (%d): Fn2: Running......\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);	
	thread_exit(42);
}

void fn3(ulong_t arg){
	first_response("fn3");
	busy_wait(JOB_DELAY);
	cons_printf("PID %d (%d): Fn3: Running.\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d (%d): Fn3: Running..\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d (%d): Fn3: Running...\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d (%d): Fn3: Running....\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d (%d): Fn3: Running.....\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d (%d): Fn3: Running......\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d (%d): Fn3: Running.......\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d (%d): Fn3: Running........\n", g_current->process->pid, g_current->process->priority);
	busy_wait(JOB_DELAY);
	thread_exit(42);
}

void interrupt_toggler(ulong_t arg){
	u32_t i;
	first_response("Important process");
	busy_wait(JOB_DELAY);
	cons_printf("PID %d: Important process: Running.\n", g_current->process->pid);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d: Important process: Running..\n", g_current->process->pid);
	busy_wait(JOB_DELAY);
	bool iflag = int_begin_atomic();
	cons_write("\nInterrupts disabled!\n");
	cons_printf("PID %d: Important process: Entering critical section...\n", g_current->process->pid);
	cons_printf("PID %d: Counting to %lu\n", g_current->process->pid, BIG_NUM);
	for (i = 1; i <= BIG_NUM; ++i)
		if ((i % BIG_INTERVAL) == 0)
			cons_printf("PID %d: %lu\n", g_current->process->pid, i);
	cons_printf("PID %d: Done!\n", g_current->process->pid);
	cons_printf("PID %d: Important process: Exiting critical section...\n", g_current->process->pid);
	cons_write("Interrupts enabled!\n\n");
	int_end_atomic(iflag);
	cons_printf("PID %d: Important process: Running...\n", g_current->process->pid);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d: Important process: Running....\n", g_current->process->pid);
	busy_wait(JOB_DELAY);
	thread_exit(42);
}

//use the peterson solution to the critical section problem to play a game of tick tack toe
void peterson_player(ulong_t arg) {
	first_response("Player");
	char c = join_tictactoe_game();
	while (*g_tictactoe_status != GAME_DONE)
		peterson(&play_tictactoe, &wait_for_turn, &s_peterson, (c == 'X') ? P_I : P_J);
	thread_exit(42);
}

//use a mutex to manage a group of threads seaking to write to the same data structure
//(all of the threads are attempting to write to the s_poem struct defined in poetry.c)
void mutex_poet(ulong_t arg) {
	first_response("Poet");
	mutex_lock(&s_job_mutex);
	write_poem();
	mutex_unlock(&s_job_mutex);
	thread_exit(42);
}
