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


void FCFSfindTurnAroundTime(const int processes[], int n, const int bt[]) {
	int wt[n], tat[n], total_wt = 0, total_tat = 0;

	// waiting time for first process is the arrival time of the process
	wt[0] = processes[0];

	// calculating waiting time
	for (int i = 1; i < n; i++)
		wt[i] = bt[i - 1] + wt[i - 1];

	// calculating turnaround time by adding: bt[i] + wt[i]
	for (int i = 0; i < n; i++) {
		tat[i] = bt[i] + wt[i] - processes[i];

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
	int leftovers[n], finish = 0;
	int processes_cpy[n];

	for (int i = 0; i < n; i++) processes_cpy[i] = processes[i];
	for (int i = 0; i < n; i++) leftovers[i] = processes[i];

	// waiting time for first process is the arrival time of the process
	wt[0] = 0;
	tat[0] = bt[0];
	processes_cpy[0] = -1;

	int last_idx = 0;

	while (finish != 1) {
		int idx = getIdxLCFSnonPre(processes, n, bt);

		// calculating waiting time
		wt[idx] = bt[last_idx] + wt[last_idx] + leftovers[last_idx] - processes[idx];

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

void swap(int*a,int*b) {
	*a = *a ^ *b;
	*b = *a ^ *b;
	*a = *b ^ *a;
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
	processes = (int *)malloc(n_procs * sizeof(int));
	burst_time = (int *)malloc(n_procs * sizeof(int));
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
	printf("before sort\n");
	for (int i = 0; i < n_procs; i++) {
		printf("%d,%d\n", processes[i], burst_time[i]);
	}
	// TODO: need to sort the processes list by arrival time
	for (int i = 0; i < n_procs; i++) {
		for (int j = i+1; j < n_procs; j++) {
			if (processes[i] > processes[j]) {
				swap(&processes[i], &processes[j]);
				swap(&burst_time[i], &burst_time[j]);
			}
		}
	}
	printf("after sort\n");
	for (int i = 0; i < n_procs; i++) {
		printf("%d,%d\n", processes[i], burst_time[i]);
	}
	// FCFS: mean turnaround = ?
	FCFSfindTurnAroundTime(processes, n_procs, burst_time);

	// LCFS (NP): mean turnaround = ?
	LCFSfindTurnAroundTime(processes, n_procs, burst_time, 0);

	// LCFS (P): mean turnaround = ?
	LCFSfindTurnAroundTime(processes, n_procs, burst_time, 1);

//	// RR: mean turnaround = ?
//	RRfindTurnAroundTime(processes, n_procs, burst_time);

//	// SJF: mean turnaround = ?
//	SJFfindTurnAroundTime(processes, n_procs, burst_time);

	free(processes);
	free(burst_time);
	return 0;
}

