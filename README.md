# Scheduler

This project aims to emulate the scheduling behavior of a single-core processor utilizing various scheduling algorithms, including **Highest Priority First (HPF)**, **Shortest Remaining Time Next (SRTN)**, and **Round-Robin with Customizable Quantum**. The project is designed to read input data from a file provided by the user, representing processes with information such as arrival time, priority, and burst time. Based on the selected scheduling algorithm input by the user, the simulator will schedule these processes accordingly.

### Highest Priority First (HPF)
Schedules processes based on their priority, with the highest priority process executed first.

### Shortest Remaining Time Next (SRTN)
Selects the process with the shortest remaining burst time for execution.

### Round-Robin with Customizable Quantum
Implements a round-robin scheduling algorithm where each process is executed for a predefined quantum of time which is customizable by the user.

Upon completion of the simulation, the project generates output files describing the timeline of the program and statistics. The `scheduler.log` file details the timeline of process execution, including start, stop, finish, and resumption times. The `scheduler.perf` file presents performance metrics such as CPU utilization, average turnaround time (WTA), average waiting time, and standard deviation of WTA. The `memory.log` file details the allocations and deallocations of the processes during the simulations.