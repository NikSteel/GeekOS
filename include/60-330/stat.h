#ifndef STAT_H
#define STAT_H

#include <stdbool.h>
#include <geekos/types.h>

#define THROUGHPUT_QUANTUM 20

struct time_stat {
	u32_t arrival;
	u32_t first_response;
	u32_t running;
	u32_t termination;
};

struct simulation_stat {
	int pid;
	u32_t wait;
	u32_t response;
	u32_t turnaround;
};

u32_t calculate_wait_time(struct time_stat);
u32_t calculate_turnaround_time(struct time_stat);
u32_t calculate_response_time(struct time_stat);

struct simulation_stat calculate_simulation_stat(struct time_stat, int);
struct simulation_stat calculate_simulation_stat_average(struct simulation_stat*, size_t);
void print_simulation_stat_table(struct simulation_stat* statlist, size_t length, struct simulation_stat average);

bool ask_for_stats(void);

//prints a message to indicate that the process has started and
//sets the first response parameter of the time_stat
void first_response(char *);

#endif