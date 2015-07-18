/*
 * GeekOS - timer device
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

#ifndef GEEKOS_TIMER_H
#define GEEKOS_TIMER_H

#include <geekos/types.h>

/* generic functions */
void timer_process_tick(void);

/* architecture-dependent functions */
void timer_init(void);

/* global tick counter */
extern volatile u32_t g_numticks;

//ADDED: global quantum value
extern volatile int g_quantum;

//MOVED from main.c
void busy_wait(u32_t);

#endif /* ifndef GEEKOS_TIMER_H */
