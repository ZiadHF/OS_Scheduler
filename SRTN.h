#include "MinHeap.h"
#include "headers.h"
#include <math.h>
#include "time.h"

/**
 * @brief Clears the contents of the log file named "scheduler.log"
 *
 * @return void
 */
void clearLogFileSRTN()
{
    FILE *filePointer;
    filePointer = fopen("scheduler.log", "w");
    if (filePointer == NULL)
    {
        printf("Unable to open scheduler.log.\n");
        return;
    }

    fclose(filePointer);
}
/**
 * @brief Logs the start time and details of a process to the "scheduler.log" file
 *
 * @param proc The process structure containing details of the process being logged
 * @return void
 */
void LogStartedSRTN(struct process *proc, int *shared)
{
    if (proc == NULL)
    {
        return;
    }
    int clock = getClk();

    FILE *filePointer;
    filePointer = fopen("scheduler.log", "a");
    *shared = proc->id;
    if (filePointer == NULL)
    {
        printf("Unable to open scheduler.log.\n");
        return;
    }
    if (proc->remainingtime != proc->runningtime)
    {
        fprintf(filePointer, "At time %d, process %d resumed. Arr: %d, remain: %d,Total:%d, wait: %d.\n",
                clock, proc->id, proc->arrivaltime, proc->remainingtime, proc->runningtime, clock - proc->arrivaltime - proc->runningtime + proc->remainingtime);
    }
    else
    {
        fprintf(filePointer, "At time %d, process %d started. Arr: %d, remain: %d,Total:%d, wait: %d.\n",
                clock, proc->id, proc->arrivaltime, proc->remainingtime, proc->runningtime, clock - proc->arrivaltime);
    }

    fclose(filePointer);
}

/**
 * @brief Logs the finish time, turnaround time, and details of a process to the "scheduler.log" file
 *
 * @param proc The process structure containing details of the finished process
 * @param noOfProcesses The total number of processes
 * @return void
 */
void LogFinishedSRTN(struct process *proc, int noOfProcesses, int *runningTimeSum, float *WTASum, int *waitingTimeSum, float TAArray[], int *TAArrayIndex,int*shared)
{
    if (proc == NULL)
    {
        printf("NULL");
        return;
    }
    int clock = getClk();
    float wta;
    FILE *filePointer;
    filePointer = fopen("scheduler.log", "a");
    if (filePointer == NULL)
    {
        printf("Unable to open scheduler.log.\n");
        return;
    }
    if (proc->remainingtime == 0)
    {
        wta = ((float)clock - proc->arrivaltime) / (float)proc->runningtime;
        fprintf(filePointer, "At time %d, process %d finished. Arr: %d, remain: %d,Total:%d, wait: %d. TA %d WTA %.2f\n",
                clock, proc->id, proc->arrivaltime, proc->remainingtime, proc->runningtime, clock - proc->arrivaltime - proc->runningtime, clock - proc->arrivaltime, wta);
        *runningTimeSum += proc->runningtime;
        *WTASum +=wta;
        *waitingTimeSum += clock - proc->arrivaltime - proc->runningtime;
        TAArray[*TAArrayIndex] = wta;
        *TAArrayIndex = *TAArrayIndex + 1;
        *shared = proc->id;
    }
    else
    {
        fprintf(filePointer, "At time %d, process %d stopped. Arr: %d, remain: %d,Total:%d, wait: %d.\n",
                clock, proc->id, proc->arrivaltime, proc->remainingtime, proc->runningtime, clock - proc->arrivaltime - proc->runningtime + proc->remainingtime);
    }
    fclose(filePointer);
}

/**
 * @brief  Receive a process from the Ready Queue
 *
 * @param minHeap  The MinHeap to insert the process in
 * @param ReadyQueueID  The ID of the Ready Queue
 * @return true
 * @return false
 */
bool ReceiveProcess(struct MinHeap *minHeap, int ReadyQueueID, struct Nodemem* root,struct process Waiting[],int* iterator,int* totalmemory,FILE* f,int GUIID)
{
    struct process ArrivedProcess;
    int received = msgrcv(ReadyQueueID, &ArrivedProcess, sizeof(ArrivedProcess), 0, IPC_NOWAIT);
    if (received != -1)
    {
        printf("Process with id %d has arrived\n", ArrivedProcess.id);
        if (AllocateMemory(root,ArrivedProcess.memsize,&ArrivedProcess,totalmemory)){ 
            msgsnd(GUIID, &ArrivedProcess, sizeof(ArrivedProcess), IPC_NOWAIT);
            char RunningTimeStr[12];
            sprintf(RunningTimeStr, "%d", ArrivedProcess.runningtime);
            char *args[3] = {"./process.out", RunningTimeStr, NULL};
            pid_t pid = fork();
            if (pid == 0) {execv(args[0], args); }
            ArrivedProcess.pid = pid;
            printf("At time %d allocated %d bytes for process %d\n",getClk(),ArrivedProcess.memsize,ArrivedProcess.id);
            MemoryLogger(1,root,&ArrivedProcess,f);
            insertSRTN(minHeap, ArrivedProcess);
        }
        else { 
            printf("Could not allocate memory for process %d with memory %d\n",ArrivedProcess.id,ArrivedProcess.memsize);
            Waiting[*iterator] = ArrivedProcess;
            *iterator += 1;
        }
        return true;
    }
    return false;
}

/**
 * @brief  Run the Shortest Remaining Time Next Algorithm
 *
 * @param noOfProcesses  The number of processes
 */
void SRTN(int noOfProcesses)
{
    FILE* f = fopen("memory.log","w");
    printf("SRTN Running\n");
    key_t runningProcKey = ftok("keys/Guirunningman", 'A');
    int runningID = shmget(runningProcKey, 4, IPC_CREAT | 0644);
    if ((long)runningID == -1)
    {
        perror("Error in creating shm!");
        exit(-1);
    }
    int *runningProcess = (int *)shmat(runningID, (void *)0, 0);
    if ((long)runningProcess == -1)
    {
        perror("Error in attaching!");
        exit(-1);
    }
    *runningProcess = -1;
    key_t deadProcKey = ftok("keys/Guideadman", 'A');
    int deadID = shmget(deadProcKey, 4, IPC_CREAT | 0644);
    if ((long)deadID == -1)
    {
        perror("Error in creating shm!");
        exit(-1);
    }
    int *deadProcess = (int *)shmat(deadID, (void *)0, 0);
    if ((long)deadProcess == -1)
    {
        perror("Error in attaching!");
        exit(-1);
    }
    *deadProcess = -1;
    float TAArray[noOfProcesses];
    int TAArrayIndex = 0;
    int iterator = 0,totalmemory = 1024;
    int runningTimeSum = 0;
    float WTASum = 0.0f;
    int waitingTimeSum = 0;
    clearLogFileSRTN();
    int remainingProcesses = noOfProcesses;
    struct Nodemem* root = InitialiseMemory(totalmemory,1);
    struct MinHeap *minHeap = createMinHeap(noOfProcesses);
    struct process* Waiting = malloc(sizeof(struct process)*noOfProcesses);
    int ReadyQueueID, SendQueueID, ReceiveQueueID,GUIID,ArrivedProcessesID;
    DefineKeys(&ReadyQueueID, &SendQueueID, &ReceiveQueueID,&GUIID,&ArrivedProcessesID);
    initSync();
    struct process *keeper = NULL;
    int clk = 0;
    while (remainingProcesses > 0)
    {
        clk = getClk();
        struct process *currentProcess = NULL, tmp;

        printf("Current clock = %d\n", clk);
        while (getSync() == 0);
        while (ReceiveProcess(minHeap, ReadyQueueID,root,Waiting,&iterator,&totalmemory,f,GUIID));
        if (minHeap->heap_size > 0)
        {
            currentProcess = getMin_ptr(minHeap);
            if (keeper == NULL)
            {
                keeper = (struct process *)malloc(sizeof(struct process));
                *keeper = *currentProcess;
                LogStartedSRTN(currentProcess, runningProcess);
            }
            if (keeper != NULL && keeper->id != currentProcess->id)
            {
                LogFinishedSRTN(keeper, noOfProcesses, &runningTimeSum, &WTASum, &waitingTimeSum, TAArray, &TAArrayIndex,deadProcess);
                LogStartedSRTN(currentProcess, runningProcess);
                *keeper = *currentProcess;
            }
            

            if (currentProcess->remainingtime <= 0)
            {
                printf("Process with ID: %d has finished\n", currentProcess->id);
                LogFinishedSRTN(currentProcess, noOfProcesses, &runningTimeSum, &WTASum, &waitingTimeSum, TAArray, &TAArrayIndex,deadProcess);
                struct process Terminated = extractMin(minHeap, 1);
                MemoryLogger(0,root,&Terminated,f);
                DeAllocateMemory(&Terminated,&totalmemory);
                remainingProcesses--;
                wait(NULL);
                CheckAllocation(1,minHeap,root,&iterator,Waiting,&totalmemory,f,GUIID);
                if (minHeap->heap_size != 0)
                {
                    currentProcess = getMin_ptr(minHeap);
                    LogStartedSRTN(currentProcess, runningProcess);
                    *keeper = *currentProcess;
                    
                }
            }
            if (minHeap->heap_size == 0)
            {
                continue;
            }
            struct msgbuff sendmsg;
            sendmsg.mtype = currentProcess->pid;
            sendmsg.msg = 1;
            // Send the turn to the current process
            int send = msgsnd(SendQueueID, &sendmsg, sizeof(sendmsg.msg), IPC_NOWAIT);
            printf("Process %d with pid = %d is running\n", currentProcess->id, currentProcess->pid);
            struct msgbuff receivedmsg;
            int received = msgrcv(ReceiveQueueID, &receivedmsg, sizeof(receivedmsg.msg), 0, !IPC_NOWAIT);
            if (received != -1)
            {
                currentProcess->remainingtime = receivedmsg.msg;
                printf("Process with id %d has received remaining time = %d\n", currentProcess->id, currentProcess->remainingtime);
            }
        }
        while (clk == getClk())
            ;
    }
    FILE *perf;
    perf = fopen("scheduler.perf", "w");
    printf("%i", runningTimeSum);
    float CPUUtilization = (float)runningTimeSum / clk * 100;
    fprintf(perf, "CPU Utilization =  %.2f %% \n", CPUUtilization);
    float AVGWTA = (float)WTASum / (float)noOfProcesses;
    fprintf(perf, "Avg WTA =  %.2f  \n", AVGWTA);
    fprintf(perf, "Avg Waiting = %.2f \n", (float)waitingTimeSum / (float)noOfProcesses);
    double counter = 0.0f;
    for (int i = 0; i < noOfProcesses; i++)
    {
        counter += (TAArray[i] - AVGWTA) * (TAArray[i] - AVGWTA);
    }

    counter = counter / noOfProcesses;
    counter = sqrt(counter);
    fprintf(perf, "Std WTA = %.2f \n", counter);
    fclose(perf);
    free(keeper);
    shmdt(runningProcess);
    shmdt(deadProcess);
    destroy(minHeap);
    ClearMemory(root);
    free(Waiting);
    fclose(f);
}