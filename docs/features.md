
#Here are some more details about our OS's features and their implementation.

##5-State Process Model

GEEKOS comes with a basic implementation of a state process model. We extended 
the provided process model to include such states as new state. Also, when the 
long term scheduler creates new kernel threads for a submitted job these are 
inserted into a new process queue (which we added) and when all new jobs have received 
a kernel thread   then all new threads are dequeued from the new queue and inserted into 
the ready queue, in a single atomic instruction. See: `include/60-330/geekos/thread.h`

Furthermore, GEEKOS manages the wait queue using the work queue process. This is defined 
by GEEKOS in the workqueue.c and work queue process is constantly running to manage I/O 
interrupts and wait queues.

##Job Submission

Our team's job submission header file contains information such as priority of the job and 
CPU burst time which is useful information to our scheduler. Our header file is located in:
`include/60-330/job.h`

##Machine Architecture

The operating system implemented uses the RAM memory model as required. We also made use 
of the I/O framework provided to us by GEEKOS; this includes the keyboard and monitor output. 
In the construction of the OS we assumed 32-bit architecture and utilized the timer provided 
with GEEKOS (timer.c). Modifications were made in the timer.c file, such as adding a modifiable 
global time quantum value.

GEEKOS also provides a mechanism for disk I/O; however, we did not incorporate this into our project.

##Process and Resource tables; Queues and Management

GEEKOS comes built with excellent device management; therefore, our project didn't redefine the 
structures provided.

The GEEKOS incorporates many of the basic requirements for a PCB into its thread structure; thus,
we simply extended the thread structure (thread.h )to incorporate other PCB features such as a 
process ID, priority, and time statistics.

A new job is allocated a kernel thread and inserted into the new thread queue (which our group 
implemented). When all new jobs have been allocated a kernel thread, the long term schedules moves 
them into the ready queue which is defined in the thread.c by GEEKOS. The management of the ready 
queue is performed by the schedulers we defined in scheduler.c.

There is only a single running process at any given time. A pointer to this process is stored in 
GEEKOS in the global variable g_current.

In GEEKOS, we are provided a work queue and a thread wait queue. When a thread needs I/O it is 
added to a wait queue. To manage the work queue and the wait queue GEEKOS initializes a work queue 
thread; the work queue thread waits for an interrupt triggered I/O event to be added to a queue of 
work items, the work queue, defined in workqueue.c. The workqueue thread calls the work item's 
callback function to send the relevant data to a process in the wait queue that is expecting the 
data and move the process out of the wait queue and into the ready queue. To clarify, the work queue 
thread operates independently of the interrupt handler, which is responsible for reporting new work 
to the work queue thread.

Terminated threads are not stored in a queue. They are scheduled for destruction using the work 
queue thread mechanism described above.

The keyboard has its own queue defined in queue.h and keyboard.c to be used in the event that too
many key entries are received for the work queue thread to manage effectively. This is one example 
of a device queue in GEEKOS we utilized in our project.

Furthermore, the dev.c file included with GEEKOS features a structure for defining and listing devices. 
We did not utilize this in our OS.

##Scheduling

The operating system implemented allow users to select from a list of schedulers (FCFS, SJF, RR, and 
Priority). Once selected it then prompts the user to select from various options for jobs (All short,
All long, mixed, and random). The jobs are then run on the scheduler previously selected by the user.

The various short term schedulers are implemented in: src/60-330/scheduler.c
The jobs described above are located in: src/60-330/simulation.c
The longterm scheduler that loads a list of new jobs into memory. See src/60-330/simulation.c

Our OS interface also prompts the user if they want to see run time statistics after a series of 
processes have finished executing based on the scheduler selected. Runtime statics are displayed 
in a table format and include information such as average waiting time, average response time, 
and average turnaround time.

This information makes comparing scheduling algorithms a more manageable task.

##User Processes Synchronization and Simulation

We have implemented single process execution and multiprogramming (ie. Multiple jobs submission and RR).

We have implemented Peterson’s solution to the critical sections problem; our implementation is located 
in src/60-330/peterson.c and include/60-330/peterson.h.  The tic tac toe simulation helps to demonstrate 
the effectiveness of synchronizing two competing processes.

GEEKOS includes functionality to momentarily disable pre-emption and interrupts using the functions 
int_begin_atomic and int_end_atomic which set the value of the CPU register eflags, defined in x86_cpu.c 
(given by GEEKOS), to true or false. When eflags is true, interrupts are enabled, and when false, they are 
disabled, permitting the calling process to complete an instruction atomically.

GEEKOS also includes an implementation of a mutex, defined in src/kernel/synch.c and include/geekos/synch.h.
We demonstrate the effectiveness of the mutex in controlling a group of competing processes with simulation 
of multiple threads attempting to write a poem concurrently.  The mutex automatically moves the competing 
threads to a wait queue and dequeues them when the lock is freed.

##User Interface and Reporting

Our OS has a command line interface with a menu. The menu can be seen by viewing the file: 
`src/60-330/menu.c`

Our OS also displays information about the current running process including when a process is started 
and running. The process ID and priority are also displayed to the terminal making it easy to track how 
the schedulers are working.
