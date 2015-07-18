#ifndef MENU_H
#define MENU_H

#include <60-330/scheduler.h>
#include <60-330/job.h>

void main_menu(void);
scheduler_t get_scheduler(void);
int get_quantum(void);
void get_synch_simulation(const struct job**, size_t *);
void get_scheduler_simulation(const struct job**, size_t *);

#endif