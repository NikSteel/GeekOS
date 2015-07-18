#include <stdbool.h>
#include <geekos/kassert.h>
#include <geekos/int.h>
#include <geekos/timer.h>
#include <geekos/cons.h>
#include <geekos/keyboard.h>
#include <geekos/thread.h>
#include <60-330/stat.h>

u32_t calculate_wait_time(struct time_stat time) {
	KASSERT((time.termination != 0) && (time.arrival != 0) && (time.running != 0));
	return time.termination - time.arrival - time.running;
}

u32_t calculate_turnaround_time(struct time_stat time) {
	KASSERT((time.termination != 0) && (time.arrival != 0));
	return time.termination - time.arrival;
}

u32_t calculate_response_time(struct time_stat time) {
	KASSERT((time.first_response != 0) && (time.arrival != 0));
	return time.first_response - time.arrival; 
}

struct simulation_stat calculate_simulation_stat(struct time_stat time, int pid) {
	struct simulation_stat stat;
	stat.pid = pid;
	stat.wait = calculate_wait_time(time);
	stat.response = calculate_response_time(time);
	stat.turnaround = calculate_turnaround_time(time);
	return stat;
}

bool ask_for_stats(void) {
	u16_t keycode;
	//prompt the user to determine whether or not to show stats
	cons_write("View runtime statistics? (y/n): "); 
	//wait for a valid keycode
	do { keycode = wait_for_key();
	} while ((keycode != 'y') && (keycode != 'n'));
	//print keycode
	cons_printf("%c\n", keycode);
	return keycode == 'y';
}

struct simulation_stat calculate_simulation_stat_average(struct simulation_stat* statlist, size_t length) {
	KASSERT(statlist != NULL);
	int i;
	struct simulation_stat average = {.pid = 0, .wait = 0, .response = 0, .turnaround = 0};
	for (i = 0; i < length; ++i) {
		average.wait += statlist[i].wait;
		average.response += statlist[i].response;
		average.turnaround += statlist[i].turnaround;
	}
	average.wait /= length;
	average.response /= length;
	average.turnaround /= length;
	return average;
}

//print runtime statistics in a large table
void print_simulation_stat_table(struct simulation_stat* statlist, size_t length, struct simulation_stat average) {
	// cons_write("Runtime statistics...\n");
	// busy_wait(10);
	cons_write("PID\t" "Wait Time\t" "Response\t" "Turnaround\n");
	int i;
	for (i = 0; i < length; ++i)
		cons_printf("%d\t%lu\t\t%lu\t\t%lu\n", statlist[i].pid, 
			statlist[i].wait, statlist[i].response, statlist[i].turnaround); 
			
	cons_printf("Ave:\t%lu\t\t%lu\t\t%lu\n", average.wait, average.response, average.turnaround);
}


//set a process's first response time
void first_response(char *fn_name) {
	bool iflag = int_begin_atomic();
	cons_printf("PID %d (%d): %s: Starting\n", g_current->process->pid, g_current->process->priority, fn_name);
	g_current->process->time.first_response = g_numticks;
	int_end_atomic(iflag);
}