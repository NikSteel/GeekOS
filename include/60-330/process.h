#ifndef PROCESS_H
#define PROCESS_H

struct process {
	int pid;						//process ID
	int priority;					//priority for PRIORITY and SJF
	struct time_stat time;			//time statistic for monitoring wait, response, and turnaround time
	/*
	int age;						//the age of a process for use with PRIORITY and SJF 
	*/								//(ageing was abandoned for lack of time)
};

#endif