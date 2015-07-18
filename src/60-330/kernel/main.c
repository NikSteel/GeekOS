/*
 * GeekOS entry point and main function
 * Copyright (C) 2001-2008, David H. Hovemeyer <david.hovemeyer@gmail.com>
 *
 * modified: Matthias Aechtner (2014)
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

//Modified by Nik, Todd, Aaron, and Chris

#include <geekos/boot.h>
#include <geekos/version.h>
#include <geekos/cons.h>
#include <geekos/mem.h>
#include <geekos/vm.h>
#include <geekos/int.h>
#include <geekos/irq.h>
#include <geekos/thread.h>
#include <geekos/workqueue.h>
#include <geekos/timer.h>
#include <geekos/ramdisk.h>
#include <geekos/blockdev_pager.h>
#include <geekos/keyboard.h>

#include <arch/ata.h>

//ADDED include menu.h
#include <60-330/menu.h>

//REMOVED dummy threads test_thread() and busy_thread()
//MOVED busy_wait() to timer.c

void geekos_main(u32_t loader_magic, struct multiboot_info *boot_record)
{
	//REMOVED keycode variable
	struct vm_pager *vmp;
	struct blockdev *ramdsk;
	char ramdsk_buf[1024];
	
	/* Initialize kernel */
	mem_clear_bss();
	cons_init();
	cons_clear();
	PANIC_IF(loader_magic != MB_LOADER_MAGIC, "Unrecognized magic");
	cons_printf("GeekOS %d.%d.%d on %s\n", GEEKOS_MAJOR, GEEKOS_MINOR, GEEKOS_PATCH, GEEKOS_ARCH);
	mem_init(boot_record);
	int_init();
	vm_init_paging(boot_record);
	irq_init();
	thread_init();
	workqueue_init();
	ata_init();
	timer_init();	
	ramdsk = ramdisk_create(ramdsk_buf, 1024);
	cons_printf("Created block device pager .....%s\n",
			blockdev_pager_create(ramdsk, lba_from_num(0), 2, &vmp) ?
			" [Failed]" : ".... [OK]");
	keyboard_init();
	//ADDED: mutex initializer
	s_job_mutex_init();

	//ADDED: greeting
	cons_write("\nWelcome to Todd, Chris, Aaron, and Nik's OS!\n");
	
	//ADDED: interactive interface
	//Due to GeekOS's restrictions on memory allocation,
	//Our OS simulation may only produce a maximum of 30 threads,
	//so this hard upper bound allows for graceful degradation.
	while(g_num_threads < MAX_THREADS) {
		main_menu();
	}

	cons_write( "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n"
				"\tThank You for using GeeokOS!\n"
				"\tPlease reboot to run more simulations\n\n");
	HALT();
	//REMOVED: timer/thread/keyboard/cons demonstrations

#if 0
	HALT();
#endif
}
