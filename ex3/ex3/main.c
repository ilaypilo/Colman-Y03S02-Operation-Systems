#pragma once
/*
Operation Systems - Igor Rochlin
27/04/19
Ex3 - Scheduler
Bar Zrihan 203285770 בר זריהן
Ilay Pilosof 304961519 עילי פילוסוף
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024


void swap(int *a, int *b) {
	*a = *a ^ *b;
	*b = *a ^ *b;
	*a = *b ^ *a;
}


int readline(int fd, char *buffer) {
	int res;
	int count = 0;
	char *ptrLine = buffer;
	while ((res = read(fd, ptrLine, 1)) > 0) {
		// break if EOL (end of line)
		if (*ptrLine == '\n' || *ptrLine == '\0') {
			// null terminate the file
			*ptrLine = '\0';
			break;
		}
		++ptrLine;
		++count;
	}
	if (res < 0) /* means file read failed */
	{
		// printf("error reading the line\n");
		_exit(res);
	}
	// printf("done reading the line: %s\n", buffer);
	return count;
}


// Function to calculate turn around time
void findTurnAroundTime(int processes[], int n, int bt[], int wt[], int tat[]) {
	// calculating turnaround time by adding
	// bt[i] + wt[i]
	for (int i = 0; i < n; i++)
		tat[i] = bt[i] + wt[i] - processes[i];
}


void FCFSfindTurnAroundTime(const int processes[], int n, const int bt[]) {
	int wt[n], tat[n], total_wt = 0, total_tat = 0;

	// waiting time for first process is the arrival time of the process
	wt[0] = processes[0];

	// calculating waiting time
	for (int i = 1; i < n; i++)
		wt[i] = bt[i - 1] + wt[i - 1];

	// calculating turnaround time by adding: bt[i] + wt[i]
	findTurnAroundTime(processes, n, bt, wt, tat);
	for (int i = 0; i < n; i++) {
		// Calculate total turn around time
		total_tat = total_tat + tat[i];
	}
	float t = (float) total_tat / (float) n;
	printf("FCFS: mean turnaround = %.2f\n", t);
}


int getIdxLCFSnonPre(const int processes[], int n, const int bt[]) {
	int idx = -1;
	int max = 0;

	for (int i = 0; i < n; i++) {
		if (processes[i] > max) {
			max = processes[i];
			idx = i;
		}
	}
	return idx;
}


void LCFSfindTurnAroundTime(const int processes[], int n, const int bt[], int preemptive) {
	int wt[n], tat[n], total_wt = 0, total_tat = 0;
	int r_queue[n], finish = 0;
	int processes_cpy[n];

	for (int i = 0; i < n; i++) processes_cpy[i] = processes[i];
	for (int i = 0; i < n; i++) r_queue[i] = processes[i];

	// waiting time for first process is the arrival time of the process
	wt[0] = 0;
	tat[0] = bt[0];
	processes_cpy[0] = -1;

	int last_idx = 0;

	while (finish != 1) {
		int idx = getIdxLCFSnonPre(processes_cpy, n, bt);

		// calculating waiting time
		wt[idx] = bt[last_idx] + wt[last_idx] + r_queue[last_idx] - processes_cpy[idx];

		// calculating turnaround time by adding: bt[i] + wt[i]
		tat[idx] = bt[idx] + wt[idx];

		last_idx = idx;
		processes_cpy[idx] = -1;
		finish = 1;

		for (int x = 0; x < n; x++) {
			// check if all processes are handled, stop the while loop.
			if (processes_cpy[x] != -1) finish = 0;
		}
	}
	// Calculate total turn around time
	for (int i = 0; i < n; i++) total_tat = total_tat + tat[i];

	float t = (float) total_tat / (float) n;

	if (preemptive == 1) printf("LCFS (P): mean turnaround = %.2f\n", t);
	else if (preemptive == 0) printf("LCFS (NP): mean turnaround = %.2f\n", t);
}


// Function to find the waiting time for all
// processes
void RRfindWaitingTime(int processes[], int n, int bt[], int wt[], int quantum) {
	// Make a copy of burst times bt[] to store remaining
	// burst times.
	int rem_bt[n];
	for (int i = 0; i < n; i++)
		rem_bt[i] = bt[i];

	int t = 0; // Current time

	// Keep traversing processes in round robin manner
	// until all of them are not done.
	while (1) {
		int done = 1;

		// Traverse all processes one by one repeatedly
		for (int i = 0; i < n; i++) {
			// If burst time of a process is greater than 0
			// then only need to process further
			if (rem_bt[i] > 0) {
				done = 0; // There is a pending process

				if (rem_bt[i] > quantum) {
					// Increase the value of t i.e. shows
					// how much time a process has been processed
					t += quantum;

					// Decrease the burst_time of current process
					// by quantum
					rem_bt[i] -= quantum;
				}

					// If burst time is smaller than or equal to
					// quantum. Last cycle for this process
				else {
					// Increase the value of t i.e. shows
					// how much time a process has been processed
					t = t + rem_bt[i];

					// Waiting time is current time minus time
					// used by this process
					wt[i] = t - bt[i];

					// As the process gets fully executed
					// make its remaining burst time = 0
					rem_bt[i] = 0;
				}
			}
		}

		// If all processes are done
		if (done == 1)
			break;
	}
}


void RRfindTurnAroundTime(const int processes[], int n, const int bt[], int quantum) {
	int wt[n], tat[n], total_wt = 0, total_tat = 0;

	// Function to find waiting time of all processes
	RRfindWaitingTime(processes, n, bt, wt, quantum);

	// Function to find turn around time for all processes
	findTurnAroundTime(processes, n, bt, wt, tat);

	// Display processes along with all details
//	printf("Processes\tBurst time\tWaiting time\tTurn around time\n");

	// Calculate total waiting time and total turn around time
	for (int i = 0; i < n; i++) {
		total_wt = total_wt + wt[i];
		total_tat = total_tat + tat[i];
//		printf("%d \t\t %d \t\t %d \t\t %d\n", i + 1, bt[i], wt[i], tat[i]);

	}


	printf("RR: mean turnaround = %.2f\n", (float) total_tat / (float) n);
}


int main(int argc, char *argv[]) {
	int fd1;  // Input file descriptor
	int n_procs;  // number of processes
	int *processes;  // arrival time list
	int *burst_time;  // Burst time of all processes
	char *proc;
	char inputLineContent[BUFFER_SIZE];
	if (argc != 2) {
		printf("Usage: %s <input file>\n", argv[0]);
		return 0;
	}

	// read config file
	fd1 = open(argv[1], O_RDONLY);
	if (fd1 < 0) {
		// printf("error open config file %s\n", argv[1]);
		return 0;
	}
	// read the first line to indicates how many processes should be
	readline(fd1, inputLineContent);
	n_procs = atoi(inputLineContent);
	// printf("Number of processes from input file: '%s' are: %d\n", argv[1], n_procs);

	// create the array of numbers from file
	processes = (int *) malloc(n_procs * sizeof(int));
	burst_time = (int *) malloc(n_procs * sizeof(int));
	for (int i = 0; i < n_procs; i++) {
		readline(fd1, inputLineContent);
		// input string line is: "a,b"
		// Returns first token
		proc = strtok(inputLineContent, ",");
		processes[i] = atoi(proc);
		proc = strtok(NULL, ",");
		burst_time[i] = atoi(proc);
		// printf("%d burst-time of %d\n", processes[i], burst_time[i]);
	}

	close(fd1);  // config file is not needed anymore, closing it
//	printf("before sort\n");
//	for (int i = 0; i < n_procs; i++) {
//		printf("%d,%d\n", processes[i], burst_time[i]);
//	}
	// TODO: need to sort the processes list by arrival time
	for (int i = 0; i < n_procs; i++) {
		for (int j = i + 1; j < n_procs; j++) {
			if (processes[i] > processes[j]) {
				swap(&processes[i], &processes[j]);
				swap(&burst_time[i], &burst_time[j]);
			}
		}
	}
//	printf("after sort\n");
//	for (int i = 0; i < n_procs; i++) {
//		printf("%d,%d\n", processes[i], burst_time[i]);
//	}
	// FCFS: mean turnaround = ?
	FCFSfindTurnAroundTime(processes, n_procs, burst_time);

	// LCFS (NP): mean turnaround = ?
	LCFSfindTurnAroundTime(processes, n_procs, burst_time, 0);

	// LCFS (P): mean turnaround = ?
//	LCFSfindTurnAroundTime(processes, n_procs, burst_time, 1);

	// RR: mean turnaround = ?
	RRfindTurnAroundTime(processes, n_procs, burst_time, 2);

//	// SJF: mean turnaround = ?
//	SJFfindTurnAroundTime(processes, n_procs, burst_time);

	free(processes);
	free(burst_time);
	return 0;
}
