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
#include <limits.h>

#define BUFFER_SIZE 1024


typedef struct _Process {
	int pid; // Process ID
	int bt; // Burst Time
	int art; // Arrival Time
} Process;


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


void FCFSfindTat(const int processes[], const int n, const int bt[], const int wt[], int tat[]) {
	// calculating turnaround time by adding
	// bt[i] + wt[i]
	for (int i = 0; i < n; i++)
		tat[i] = bt[i] + wt[i] - processes[i];
}


void FCFSfindWaitingTime(const int processes[], const int n, const int bt[], int wt[]) {
	// waiting time for first process is 0
	wt[0] = processes[0];

	// calculating waiting time
	for (int i = 1; i < n; i++)
		wt[i] = bt[i - 1] + wt[i - 1];
}


void FCFSfindTurnAroundTime(const int processes[], int n, const int bt[]) {
	int wt[n], tat[n], total_wt = 0, total_tat = 0;

	//Function to find waiting time of all processes
	FCFSfindWaitingTime(processes, n, bt, wt);

	//Function to find turn around time for all processes
	FCFSfindTat(processes, n, bt, wt, tat);

	// Calculate total waiting time and total turn around time
	for (int i = 0; i < n; i++) {
		total_wt = total_wt + wt[i];
		total_tat = total_tat + tat[i];
	}
	float current_time = (float) total_tat / (float) n;
	printf("FCFS: mean turnaround = %.2f\n", current_time);
}


// A structure to represent a queue
struct Queue {
	int front, rear, size;
	unsigned capacity;
	int *array;
};

// function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue *createQueue(unsigned capacity) {
	struct Queue *queue = (struct Queue *) malloc(sizeof(struct Queue));
	queue->capacity = capacity;
	queue->front = queue->size = 0;
	queue->rear = capacity - 1;  // This is important, see the enqueue
	queue->array = (int *) malloc(queue->capacity * sizeof(int));
	return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue *queue) { return (queue->size == queue->capacity); }

// Queue is empty when size is 0
int isEmpty(struct Queue *queue) { return (queue->size == 0); }

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue *queue, int item) {
	if (isFull(queue))
		return;
	queue->rear = (queue->rear + 1) % queue->capacity;
	queue->array[queue->rear] = item;
	queue->size = queue->size + 1;
//	printf("%d enqueued to queue\n", item);
}

// Function to remove an item from queue.
// It changes front and size
int dequeue(struct Queue *queue) {
	if (isEmpty(queue))
		return INT_MIN;
	int item = queue->array[queue->front];
	queue->front = (queue->front + 1) % queue->capacity;
	queue->size = queue->size - 1;
//	printf("%d dequeue from queue\n", item);
	return item;
}


// Function to get front of queue
int isExists(struct Queue *queue, int x) {
	if (isEmpty(queue))
		return 1;
	for (int i = 0; i < queue->capacity; i++) {
		if (queue->array[i] == x) return 0;
	}
	return 1;
}



void LCFSfindWaitingTime(const int processes[], const int n, const int bt[], int wt[]) {
	// waiting time for first process is 0
	wt[0] = processes[0];

	// calculating waiting time
	for (int i = 1; i < n; i++)
		wt[i] = bt[i - 1] + wt[i - 1];
}


void LCFSfindTurnAroundTime(struct Queue* queue, const int processes[], int n, const int bt[], int preemptive) {
	int wt[n], tat[n], total_wt = 0, total_tat = 0;
	// Make a copy of burst times bt[] to store remaining
	// burst times.
	int rem_bt[n];
	for (int i = 0; i < n; i++)
		rem_bt[i] = bt[i];

	int current_time = 0; // Current time

	// Keep traversing processes until all of them are done.
	while (1) {
		int done = 1;
		current_time++;

		for (int i = 0; i < n; i++)
		{
			// go over all processes, if arrival time is equal to current time, add process to queue.
			if (processes[i] == current_time) {
				if (isExists(queue, i)) enqueue(queue, i);
			}
		}

		for (int j=0; j < n; j++) {
			if (rem_bt[j] != 0) done = 0;
		}

		// If all processes are done
		if (isEmpty(queue)) {
			if (done == 1) break;
			continue;
		}
//		printf("current_time is: %d\n", current_time);

		int i = dequeue(queue);
//		printf("current process index is: %d\n", i);

		if (preemptive == 0) {
			rem_bt[i] = 0;

			// calculating waiting time
			LCFSfindWaitingTime(processes, n, bt, wt);

			// calculating turnaround time by adding: bt[i] + wt[i]
			tat[i] = bt[i] + wt[i];

			for (int x = 0; x < n; x++)
			{
				// go over all processes, if arrival time is equal to current time, add process to queue.
				if (processes[x] > current_time && processes[x] < current_time + bt[i]) {
					if (isExists(queue, x)) enqueue(queue, x);
				}
			}
			current_time += bt[i] - 1;
		}
		else {
			//preemptive
			int minus = 0;

			int arrive = -1;

			// check if new process arrive
			for (int t = 0; t < bt[i]; t++) {
				for (int x = 0; x < n; x++)
				{
					// go over all processes, if arrival time is equal to current time, add process to queue.
					if (processes[x] > current_time && processes[x] <= current_time + t) {
						arrive = x;
						minus = t;
						break;
					}
				}
				if (minus != 0) {
					break;
				}
			}

			if (arrive != -1 && rem_bt[i] > minus) {
				// means that some process arrive
				rem_bt[i] -= minus;
				for (int x = 0; x < n; x++)
				{
					// go over all processes, if arrival time is equal to current time, add process to queue.
					if (processes[x] >= current_time && processes[x] < current_time + rem_bt[i]) {
						if (isExists(queue, x)) enqueue(queue, x);
					}
				}
				if (rem_bt[0] != 0) enqueue(queue, i);
				current_time += minus - 1;
			} else {
				rem_bt[i] = 0;
//				printf("Process index %d is completed\n", i);

				for (int x = 0; x < n; x++)
				{
					// go over all processes, if arrival time is equal to current time, add process to queue.
					if (processes[x] > current_time && processes[x] < current_time + bt[i]) {
						if (isExists(queue, x)) enqueue(queue, x);
					}
				}
				current_time += bt[i] - 1;
				tat[i] = current_time - processes[i];
			}
		}

	}
	// Calculate total turn around time
	for (int i = 0; i < n; i++) total_tat = total_tat + tat[i];

	float _current_time = (float) total_tat / (float) n;

	if (preemptive == 1) printf("LCFS (P): mean turnaround = %.2f\n", _current_time);
	else if (preemptive == 0) printf("LCFS (NP): mean turnaround = %.2f\n", _current_time);
	// clean the queue if needed
	while (!isEmpty(queue)) dequeue(queue);
}


void RRfindWaitingTime(const int processes[], int n, const int bt[], int wt[], int quantum) {
	// Make a copy of burst times bt[] to store remaining
	// burst times.
	int rem_bt[n];
	for (int i = 0; i < n; i++)
		rem_bt[i] = bt[i];

	int current_time = 0; // Current time

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
					// Increase the value of current_time i.e. shows
					// how much time a process has been processed
					current_time += quantum;

					// Decrease the burst_time of current process
					// by quantum
					rem_bt[i] -= quantum;
				}

					// If burst time is smaller than or equal to
					// quantum. Last cycle for this process
				else {
					// Increase the value of current_time i.e. shows
					// how much time a process has been processed
					current_time = current_time + rem_bt[i];

					// Waiting time is current time minus time
					// used by this process
					wt[i] = current_time - bt[i];

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


void RRfindTat(const int processes[], const int n, const int bt[], const int wt[], int tat[]) {
	// calculating turnaround time by adding
	// bt[i] + wt[i]
	for (int i = 0; i < n; i++)
		tat[i] = bt[i] + wt[i] - processes[i];
}


void RRfindTurnAroundTime(const int processes[], int n, const int bt[], int quantum) {
	int wt[n], tat[n], total_wt = 0, total_tat = 0;

	// Function to find waiting time of all processes
	RRfindWaitingTime(processes, n, bt, wt, quantum);

	// Function to find turn around time for all processes
	RRfindTat(processes, n, bt, wt, tat);

	// Display processes along with all details
//	printf("Processes\tBurst time\tWaiting time\tTurn around time\n");

	// Calculate total waiting time and total turn around time
	for (int i = 0; i < n; i++) {
		total_wt = total_wt + wt[i];
		total_tat = total_tat + tat[i];
//		printf("%d \current_time\current_time %d \current_time\current_time %d \current_time\current_time %d\n", i + 1, bt[i], wt[i], tat[i]);

	}

	printf("RR: mean turnaround = %.2f\n", (float) total_tat / (float) n);
}


void SJFfindWaitingTime(Process process_list[], int n, int wt[]) {
	int remaining_time[n];

	// Copy the burst time into remaining_time[]
	for (int i = 0; i < n; i++)
		remaining_time[i] = process_list[i].bt;

	int complete_process = 0, current_time = 0, min_bt_left = INT_MAX;
	int shortest_job_index = 0, finish_time;
	int need_to_execute = 0;

	// FIXED: complete_process should include process with bt=0
	// calculate process with bt=0
	for (int i = 0; i < n; i++) {
		if (0 == process_list[i].bt) {
			++complete_process;
		}
	}

	// Process until all processes gets
	// complete_processd
	while (complete_process != n) {

		// Find process with minimum
		// remaining time among the
		// processes that arrives till the
		// current time`
		for (int j = 0; j < n; j++) {
			if (
					process_list[j].art <= current_time &&
					remaining_time[j] < min_bt_left &&
					remaining_time[j] > 0
					) {
				min_bt_left = remaining_time[j];
				shortest_job_index = j;
				need_to_execute = 1;
			}
		}

		if (0 == need_to_execute) {
			++current_time;
			continue;
		}

		// Reduce remaining time by one
		--remaining_time[shortest_job_index];

		// Update minimum
		min_bt_left = remaining_time[shortest_job_index];
		// if job completed, reset the min count
		if (min_bt_left == 0) {
			min_bt_left = INT_MAX;
		}


		// If a process gets completely
		// executed
		if (remaining_time[shortest_job_index] == 0) {

			// Increment complete_process
			++complete_process;
			need_to_execute = 0;

			// Find finish time of current
			// process
			finish_time = current_time + 1;

			// Calculate waiting time
			wt[shortest_job_index] = finish_time -
									 process_list[shortest_job_index].bt -
									 process_list[shortest_job_index].art;

			if (wt[shortest_job_index] < 0)
				wt[shortest_job_index] = 0;
		}
		// Increment time
		current_time++;
	}
}


void SJFfindTurnAroundTime(Process process_list[], int n) {
	int wt[n], tat[n], total_wt = 0, total_tat = 0;

	// Function to find waiting time of all
	// processes
	SJFfindWaitingTime(process_list, n, wt);

	// Function to find turn around time for
	// all processes
	// calculating turnaround time by adding
	// bt[i] + wt[i]
	for (int i = 0; i < n; i++)
		tat[i] = process_list[i].bt + wt[i];

	// Display processes along with all details
//	printf("Processes\tBurst time\tWaiting time\tTurn around time\n");


	// Calculate total waiting time and
	// total turnaround time
	for (int i = 0; i < n; i++) {
		total_wt = total_wt + wt[i];
		total_tat = total_tat + tat[i];
//		printf("%d \current_time\current_time %d \current_time\current_time %d \current_time\current_time %d\n", process_list[i].pid, process_list[i].bt, wt[i], tat[i]);
	}

	printf("SJF: mean turnaround = %.2f\n", (float) total_tat / (float) n);
}


int main(int argc, char *argv[]) {
	int fd1;  // Input file descriptor
	int n_procs;  // number of processes
	int *processes;  // arrival time list
	int *burst_time;  // Burst time of all processes
	int temp;
	Process *procs;
	char *process_list;
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
	temp = n_procs;
	// printf("Number of processes from input file: '%s' are: %d\n", argv[1], n_procs);

	// create the array of numbers from file
	processes = (int *) malloc(n_procs * sizeof(int));
	burst_time = (int *) malloc(n_procs * sizeof(int));
	for (int i = 0; i < n_procs; i++) {
		readline(fd1, inputLineContent);
		// input string line is: "a,b"
		// Returns first token
		process_list = strtok(inputLineContent, ",");
		processes[i] = atoi(process_list);
		process_list = strtok(NULL, ",");
		burst_time[i] = atoi(process_list);
		if (burst_time[i] == 0) {
			temp--;
			processes[i] = INT_MAX;
		}
		// printf("%d burst-time of %d\n", processes[i], burst_time[i]);
	}

	close(fd1);  // config file is not needed anymore, closing it
//	printf("before sort\n");
//	for (int i = 0; i < n_procs; i++) {
//		printf("%d,%d\n", processes[i], burst_time[i]);
//	}
	for (int i = 0; i < n_procs; i++) {
		for (int j = i + 1; j < n_procs; j++) {
			if (processes[i] > processes[j]) {
				swap(&processes[i], &processes[j]);
				swap(&burst_time[i], &burst_time[j]);
			}
		}
	}
	n_procs = temp;
//	printf("after sort\n");
//	for (int i = 0; i < n_procs; i++) {
//		printf("%d,%d\n", processes[i], burst_time[i]);
//	}

	procs = (Process *) malloc(n_procs * sizeof(Process));
	for (int i = 0; i < n_procs; i++) {
		procs[i].pid = i;
		procs[i].art = processes[i];
		procs[i].bt = burst_time[i];
	}

	// FCFS: mean turnaround = ?
	FCFSfindTurnAroundTime(processes, n_procs, burst_time);

	struct Queue* queue = createQueue(n_procs);

	// LCFS (NP): mean turnaround = ?
	LCFSfindTurnAroundTime(queue, processes, n_procs, burst_time, 0);

	// LCFS (P): mean turnaround = ?
	LCFSfindTurnAroundTime(queue, processes, n_procs, burst_time, 1);

	// RR: mean turnaround = ?
	RRfindTurnAroundTime(processes, n_procs, burst_time, 2);

	// SJF: mean turnaround = ?
	SJFfindTurnAroundTime(procs, n_procs);

	free(queue->array);
	free(procs);
	free(processes);
	free(burst_time);
	return 0;
}
