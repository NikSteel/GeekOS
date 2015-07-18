/*
 * GeekOS - generic timer functions
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

#include <geekos/timer.h>
#include <geekos/thread.h>

//ADDED: includes
#include <60-330/scheduler.h>
#include <60-330/stat.h>

//REMOVED: geekos TIMER_QUANTUM

volatile u32_t g_numticks;

//ADDED: global time quantum value
volatile int g_quantum;

/*
 * Process a single timer tick.
 * Called from timer interrupt handler function.
 */
void timer_process_tick(void)
{
	/* update global tick counter and current thread's tick counter */
	++g_numticks;
	g_current->num_ticks++;
	
	//ADDED: increment running time
	g_current->process->time.running++;
	
	//ADDED: if RR is enabled schedule using RR_scheduler()
	if (g_scheduler == RR) {
		RR_scheduler();
	}
	
	/*
	//ADDED: Age processes in priority scheduling
	else if ((g_scheduler == SJF) || (g_scheduler == PRIORITY))
		aging();
	*/
}

//MOVED from main.c
void busy_wait(u32_t ticks)
{
	u32_t last = g_numticks;
	while (g_numticks < last + ticks) {
		/* wait */
	}
}
