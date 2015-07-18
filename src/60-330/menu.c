#include <geekos/types.h>
#include <geekos/cons.h>
#include <geekos/keyboard.h>
#include <geekos/timer.h>
#include <60-330/menu.h>
#include <60-330/scheduler.h>
#include <60-330/simulation.h>

//Choose between process scheduling and process synchronization demo
void main_menu(void) {
	u16_t keycode;
	const struct job * simulation;
	size_t length;
	
	//prompt
	cons_write(	"Enter a number to select a simulation:\n"
				"   1) Synchronization\n"
				"   2) Scheduling\n"
				"Your selection: ");
	//wait for a valid keycode
	do {
		keycode = wait_for_key();
	} while (!(('1' <= keycode) && (keycode <= '2')));
	
	//print keycode
	cons_printf("%c\n", keycode);
	
	//run the simulation associated with the selection
	switch (keycode) {
		case '1':
			//Prompt the user to select a synchronization simulation
			get_synch_simulation(&simulation, &length);
			
			break;
			
		case '2': 
			//Prompt the user to select a scheduling algorithm 
			g_scheduler = get_scheduler();
		
			//If RR, prompt the user to select a time quantum
			if (g_scheduler == RR)
				g_quantum = get_quantum();
			
			//Prompt the user to select a list of jobs to run the simulation with
			get_scheduler_simulation(&simulation, &length);
		
			break;
	}
	
	//Run the selected simulation 
	run_simulation(simulation, length);
}

//prompt the user to select a list of jobs to run the synchronization simulation with
void get_synch_simulation(const struct job** simulation, size_t* length) {
	u16_t keycode;
	
	//prompt
	cons_write(	"Enter a number to select a synchronization simulation:\n"
				"   1) Pre-emption toggle\n"
				"   2) Peterson's Solution\n"
				"   3) Mutex\n"
				"Your selection: ");
	//wait for a valid keycode
	do {
		keycode = wait_for_key();
	} while (!(('1' <= keycode) && (keycode <= '3')));
	
	//print keycode
	cons_printf("%c\n", keycode);
	
	//set the scheduler to RR with a moderate time quantum
	g_scheduler = RR;
	g_quantum = 24;
	
	//set the simulation (defined in simulation.c) using the key code as an index
	*simulation = g_synch_sim[keycode - '1'];
	*length = g_synch_sim_length[keycode - '1'];
}

//prompt the user to select a non-pre-emptive scheduler to simulate
scheduler_t get_scheduler(void) {
	u16_t keycode;
	
	//prompt
	cons_write(	"Enter a number to select a scheduler:\n"
				"   1) FCFS\n"
				"   2) SJF\n"
				"   3) Priority\n"
				"   4) RR\n"
				"Your selection: ");
	//wait for a valid keycode
	do {
		keycode = wait_for_key();
	} while (!(('1' <= keycode) && (keycode <= '4')));
	
	//print keycode
	cons_printf("%c\n", keycode);
	
	//return scheduler type associated with keycode
	switch (keycode) {
		case '1': return FCFS; 
		case '2': return SJF;
		case '3': return PRIORITY;
		case '4': return RR;
	}
	
	//this should be unreachable, but in the event of an error, default to FCFS
	return FCFS;
}

//prompt the user to select a pre-emptive scheduler to simulate
int get_quantum(void) {
	u16_t keycode;
	
	//prompt
	cons_printf("Enter a number to select a time quantum:\n"
				"   1) %d\n"
				"   2) %d\n"
				"   3) %d\n"
				"Your selection: ", RR_QUANTUM1, RR_QUANTUM2, RR_QUANTUM3);
	//wait for a valid keycode
	do {
		keycode = wait_for_key();
	} while (!(('1' <= keycode) && (keycode <= '3')));
	
	//print keycode
	cons_printf("%c\n", keycode);
	
	//return scheduler type associated with keycode
	switch (keycode) {
		case '1': return RR_QUANTUM1;
		case '2': return RR_QUANTUM2;
		case '3': return RR_QUANTUM3;
	}
	
	//this should be unreachable, but in the event of an error, default to RR_QUANTUM1
	return RR_QUANTUM1;
}

//prompt the user to select a list of jobs to run the simulation with
void get_scheduler_simulation(const struct job** simulation, size_t* length) {
	u16_t keycode;
	
	//prompt
	cons_write(	"Enter a number to select a list of jobs:\n"
				"   1) All short\n"
				"   2) All long\n"
				"   3) Mixed short and long\n"
				"   4) Randomized\n"
				"Your selection: ");
	//wait for a valid keycode
	do {
		keycode = wait_for_key();
	} while (!(('1' <= keycode) && (keycode <= '4')));
	
	//print keycode
	cons_printf("%c\n", keycode);
	
	//set the simulation (defined in simulation.c) using the key code as an index
	*simulation = g_scheduler_sim[keycode - '1'];
	*length = SCHEDULER_SIM_LENGTH;
}