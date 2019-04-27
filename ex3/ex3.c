/*
Operation Systems - Igor Rochlin
27/04/19
Ex3 - Scheduler
Bar Zrihan 203285770 בר זריהן
Ilay Pilosof 304961519 עילי פילוסוף
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int readFile(int fd, char * buffer)
{
        int readBytes;
        int totalBytes = 0;
        do
        {
                readBytes = read(fd, buffer, BUFFER_SIZE);
                if (readBytes < 0)
                {
                        // printf("error reading the file\n");
                        _exit(readBytes);
                }
                totalBytes += readBytes;

        } while (readBytes == BUFFER_SIZE);
        return totalBytes;
}

// Function to find the waiting time for all processes  
void findWaitingTime(int processes[], int n, int bt[], int wt[])  
{  
    // waiting time for first process is 0  
    wt[0] = 0;  
    
    // calculating waiting time  
    for (int  i = 1; i < n ; i++ )  
        wt[i] =  bt[i-1] + wt[i-1] ;  
}  
    
// Function to calculate turn around time  
void findTurnAroundTime( int processes[], int n, int bt[], int wt[], int tat[])  
{  
    // calculating turnaround time by adding  
    // bt[i] + wt[i]  
    for (int  i = 0; i < n ; i++)  
        tat[i] = bt[i] + wt[i];  
}  
    
//Function to calculate average time  
void findavgTime( int processes[], int n, int bt[])  
{  
    int wt[n], tat[n], total_wt = 0, total_tat = 0;  
    
    //Function to find waiting time of all processes  
    findWaitingTime(processes, n, bt, wt);  
    
    //Function to find turn around time for all processes  
    findTurnAroundTime(processes, n, bt, wt, tat);  
    
    //Display processes along with all details  
    printf("Processes   Burst time   Waiting time   Turn around time\n");  
    
    // Calculate total waiting time and total turn   
    // around time  
    for (int  i=0; i<n; i++)  
    {  
        total_wt = total_wt + wt[i];  
        total_tat = total_tat + tat[i];  
        printf("   %d ",(i+1)); 
        printf("       %d ", bt[i] ); 
        printf("       %d",wt[i] ); 
        printf("       %d\n",tat[i] );  
    }  
    int s=(float)total_wt / (float)n; 
    int t=(float)total_tat / (float)n; 
    printf("Average waiting time = %d",s); 
    printf("\n"); 
    printf("Average turn around time = %d ",t);  
}  
    
int main(int argc, char* argv[])  
{  
    int fd1;  // Input file descriptor
	char * inputFileContent[BUFFER_SIZE];
        if (argc != 2) {
              printf("Usage: %s <input file>\n" , argv[0]);
                return 0;
        }

	// read config file
        fd1 = open(argv[1], O_RDONLY);
        if (fd1 < 0)
        {
                // printf("error open config file %s\n", argv[1]);
                return 0;
        }
        // read the first 3 lines, each line indicates different path
        readFile(fd1, inputFileContent);
        close(fd1);  // config file is not needed anymore, closing it


	//process id's 
	 
    int processes[] = { 1, 2, 3};  
    int n = sizeof processes / sizeof processes[0];  
    
    //Burst time of all processes  
    int  burst_time[] = {10, 5, 8};  
    
    findavgTime(processes, n,  burst_time);  
    return 0;  
}

