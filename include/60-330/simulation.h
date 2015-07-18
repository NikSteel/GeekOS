#ifndef SIMULATION_H
#define SIMULATION_H

#define NUM_SCHEDULER_SIM 4
#define NUM_SYNCH_SIM 3
#define SCHEDULER_SIM_LENGTH 6

#include <60-330/job.h>
#include <geekos/types.h>

//Copied from src/x86/kernel/x86_keyb.c
#define KEYB_IRQ 1

//global arrays of jobs to run as a simulation
extern volatile int g_simulation_active;
extern const struct job g_scheduler_sim[NUM_SCHEDULER_SIM][SCHEDULER_SIM_LENGTH];
extern const struct job* g_synch_sim[NUM_SYNCH_SIM];
extern const size_t g_synch_sim_length[NUM_SYNCH_SIM];
extern const struct job g_preemption_toggle_sim[3];
extern const struct job g_peterson_sim[2];
extern const struct job g_mutex_sim[5];


void run_simulation(const struct job * const, size_t);
void narrator(int, char*);

#endif