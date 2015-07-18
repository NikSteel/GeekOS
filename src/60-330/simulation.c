#include <stdbool.h>
#include <geekos/timer.h>
#include <geekos/thread.h>
#include <geekos/string.h>
#include <geekos/irq.h>
#include <geekos/mem.h>
#include <60-330/simulation.h>
#include <60-330/scheduler.h>
#include <60-330/job.h>
#include <60-330/stat.h>

volatile int g_simulation_active;

//load a list of jobs into memory, wait for all jobs to terminate
//and calculate and display runtime statistics
void run_simulation(const struct job * const simulation, size_t length) {	
	//create a table to store runtime statistics
	struct simulation_stat stat[length];
	struct thread * threads[length];
	
	//disable keyboard interrupts during the simulation
	irq_disable(KEYB_IRQ);
	
	//enable print messages during thread_state transitions
	g_simulation_active = true;
	
	//submit a list of jobs to the long term scheduler
	long_term_scheduler(threads, simulation, length, THREAD_ATTACHED);
	
	//wait until all simulation threads have exited
	wait_for_children(threads, length, stat);
	
	//disable print messages during thread_state transitions
	g_simulation_active = false;
	
	//calculate runtime averages
	struct simulation_stat average =  calculate_simulation_stat_average(stat, length);
	
	//re-enable keyboard interrupts during the simulation
	irq_enable(KEYB_IRQ);
	
	//display the results:
	if (ask_for_stats())
		print_simulation_stat_table(stat, length, average);
}

//comment on process state transitions
void narrator(int pid, char* message) {
	if ((g_simulation_active) && (pid >= 3)) {
		cons_printf("Process %d changed to %s.\n", pid, message);
	}
}

//globally accessible lists of jobs which may be loaded as a simulation
//all job functions are defined in job.c
const struct job g_scheduler_sim[NUM_SCHEDULER_SIM][SCHEDULER_SIM_LENGTH] = {
	[0] = 	{ 	{.fn_ptr = &fn1, .cpu_burst = 10, .priority = 4},  
				{.fn_ptr = &fn1, .cpu_burst = 10, .priority = 3},  
				{.fn_ptr = &fn1, .cpu_burst = 10, .priority = 2},  
				{.fn_ptr = &fn1, .cpu_burst = 10, .priority = 1},  
				{.fn_ptr = &fn1, .cpu_burst = 10, .priority = 5},  
				{.fn_ptr = &fn1, .cpu_burst = 10, .priority = 6}
			},
			
	[1] = 	{ 	{.fn_ptr = &fn3, .cpu_burst = 30, .priority = 2},  
				{.fn_ptr = &fn3, .cpu_burst = 30, .priority = 3},  
				{.fn_ptr = &fn3, .cpu_burst = 30, .priority = 5},  
				{.fn_ptr = &fn3, .cpu_burst = 30, .priority = 1},  
				{.fn_ptr = &fn3, .cpu_burst = 30, .priority = 6},  
				{.fn_ptr = &fn3, .cpu_burst = 30, .priority = 4}
			},
			
	[2] = 	{ 	{.fn_ptr = &fn2, .cpu_burst = 20, .priority = 1},  
				{.fn_ptr = &fn1, .cpu_burst = 10, .priority = 6},  
				{.fn_ptr = &fn3, .cpu_burst = 30, .priority = 2},  
				{.fn_ptr = &fn1, .cpu_burst = 10, .priority = 5},  
				{.fn_ptr = &fn3, .cpu_burst = 30, .priority = 4},  
				{.fn_ptr = &fn2, .cpu_burst = 20, .priority = 3}
			},
			
	[3] = 	{ 	{.fn_ptr = &fn1, .cpu_burst = 10, .priority = 6},  
				{.fn_ptr = &fn2, .cpu_burst = 20, .priority = 5},  
				{.fn_ptr = &fn3, .cpu_burst = 30, .priority = 4},  
				{.fn_ptr = &fn1, .cpu_burst = 10, .priority = 3},  
				{.fn_ptr = &fn2, .cpu_burst = 20, .priority = 2},  
				{.fn_ptr = &fn3, .cpu_burst = 30, .priority = 1}
			}

};

const struct job* g_synch_sim[NUM_SYNCH_SIM] = {
	g_preemption_toggle_sim,
	g_peterson_sim,
	g_mutex_sim
};

const size_t g_synch_sim_length[NUM_SYNCH_SIM] = {
	3,
	2,
	5
};

const struct job g_preemption_toggle_sim[] = {
	{.fn_ptr = &fn1, 				.cpu_burst = 10, .priority = 1},  
	{.fn_ptr = &interrupt_toggler,	.cpu_burst = 10, .priority = 1},  
	{.fn_ptr = &fn1,				.cpu_burst = 10, .priority = 1}
};

const struct job g_peterson_sim[] = {
	{.fn_ptr = &peterson_player, .cpu_burst = 10, .priority = 1},  
	{.fn_ptr = &peterson_player, .cpu_burst = 10, .priority = 1}
};

const struct job g_mutex_sim[] = {
	{.fn_ptr = &mutex_poet, .cpu_burst = 10, .priority = 1},
	{.fn_ptr = &mutex_poet, .cpu_burst = 10, .priority = 1},  
	{.fn_ptr = &mutex_poet, .cpu_burst = 10, .priority = 1},  
	{.fn_ptr = &mutex_poet, .cpu_burst = 10, .priority = 1},  
	{.fn_ptr = &mutex_poet, .cpu_burst = 10, .priority = 1}
};