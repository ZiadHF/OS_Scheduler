#ifndef HEADERS_H
#define HEADERS_H

#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

typedef short bool;
#define true 1
#define false 0

#define IGNORE_LENGTH 128
#define MAX_SIZE 1024
#define SHKEY 300


///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================


/**
 * @brief Get the Clk object
 * 
 * @return int 
 */
int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/**
 * @brief  Destroy the clock (Detach/Destroy)
 * 
 * @param terminateAll  A flag to indicate whether that this is the end of simulation
 */
void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

// This is our process struct, it encapsulates all the necessary data to describe a process
struct process
{
    int id;
    pid_t pid;
    int arrivaltime;
    int runningtime;
    int priority;
    int starttime;
    int endtime;
    int remainingtime;
    int waittime;
    int responsetime;
    int finishtime;
    int turnaroundtime;
    int lasttime;
    int flag;
    int memsize,memoryused;
    struct Nodemem *mem;
};
struct msg
{
    struct process * proc; 
};
struct msgbuff
{
    long mtype;
    int msg;
};
/**
 * @brief Function to initialize a process pointer given its data
 * 
 * @param id Process id after it gets forked
 * @param arrivaltime Arrival time that was set by the process generator
 * @param runningtime The time the process needs to run
 * @param priority Takes a priority value between 1 and 10, 1 being the highest priority and 10 being the lowest
 * @param memsize The memory that the process needs.
 * @return struct process* returns a pointer to the process created
 */
struct process initializeProcess(int id, int arrivaltime, int runningtime, int priority,int memsize) {
    struct process p;
    p.id = id;
    p.arrivaltime = arrivaltime;
    p.runningtime = runningtime; // Corrected bursttime assignment
    p.priority = priority;
    p.remainingtime = runningtime;
    p.memsize = memsize;
    p.flag = 0;
    p.mem = NULL;
    return p;
}

void testerfunction(struct process* p){
    printf("%d %d %d %d %d %d",p->id,p->arrivaltime,p->runningtime,p->remainingtime,p->priority,p->memsize);
    printf("\n");
}

/**
 * @brief Skips the first line of the file
 * 
 * @param f  The file pointer to the file you want to read from
 */
void skipLine(FILE* f){
    char ignore[IGNORE_LENGTH];
    fgets(ignore,IGNORE_LENGTH,f);
}

/**
 * @brief Gets the number of processes in the file
 * 
 * @param f The file pointer to the file you want to read from
 * @return int The number of processes in the file
 */
int getnoOfProcesses(FILE* f){
    int c;
    int count = 0;
    while ((c = fgetc(f)) != EOF) { if (c == '\n') { count++; } }
    fseek(f,0,SEEK_SET);
    return count;
}

/**
 * @brief 
 * 
 * @param ReadyQueueID  The ID of the Ready Queue
 * @param SendQueueID  The ID of the Send Queue
 * @param ReceiveQueueID  The ID of the Receive Queue
 * @param GUIID  The ID of the GUI Queue
 * @param ArrivedProcessesID  The ID of the Arrived Processes Queue
 */
void DefineKeys(int* ReadyQueueID, int* SendQueueID, int* ReceiveQueueID,int* GUIID,int* ArrivedProcessesID){
    key_t ReadyQueueKey;
    ReadyQueueKey= ftok("keys/Funnyman",'A');
    *ReadyQueueID = msgget(ReadyQueueKey, 0666 | IPC_CREAT);
    if (*ReadyQueueID == -1)
    {
        perror("Error in create message queue");
        exit(-1);
    }
    //Initialize Send queue to send turn to process
    key_t SendQueueKey;
    SendQueueKey= ftok("keys/Sendman",'A');
    *SendQueueID = msgget(SendQueueKey, 0666 | IPC_CREAT);
    if (*SendQueueID == -1)
    {
        perror("Error in create message queue");
        exit(-1);
    }
    //Initialize Receive queue to receive remaining time from process
    key_t ReceiveQueueKey;
    ReceiveQueueKey= ftok("keys/Receiveman",'A');
    *ReceiveQueueID = msgget(ReceiveQueueKey, 0666 | IPC_CREAT);
    if (*ReceiveQueueID == -1)
    {
        perror("Error in create message queue");
        exit(-1);
    }
    key_t GUIKey = ftok("keys/Guiman", 'A');
    *GUIID = msgget(GUIKey, 0666 | IPC_CREAT);
    if (*GUIID == -1)
    {
        perror("Error in create message queue");
        exit(-1);
    }
    key_t ArrivedProcessesKey = ftok("keys/Guiman",'B');
    *ArrivedProcessesID = msgget(ArrivedProcessesKey, 0666 | IPC_CREAT);
    if (*ArrivedProcessesID == -1)
    {
        perror("Error in create message queue");
        exit(-1);
    }
}
/**
 * @brief  Define the keys for the message queues
 * 
 * @param SendQueueID  The ID of the Send Queue 
 * @param ReceiveQueueID  The ID of the Receive Queue
 */
void DefineKeysProcess(int* SendQueueID, int* ReceiveQueueID){

    //Initialize Send queue to send turn to process
    key_t SendQueueKey;
    SendQueueKey= ftok("keys/Sendman",'A');
    *SendQueueID = msgget(SendQueueKey, 0666 | IPC_CREAT);
    if (*SendQueueID == -1)
    {
        perror("Error in create message queue");
        exit(-1);
    }
    //Initialize Receive queue to receive remaining time from process
    key_t ReceiveQueueKey;
    ReceiveQueueKey= ftok("keys/Receiveman",'A');
    *ReceiveQueueID = msgget(ReceiveQueueKey, 0666 | IPC_CREAT);
    if (*ReceiveQueueID == -1)
    {
        perror("Error in create message queue");
        exit(-1);
    }
}

int * Synchro;                

int getSync()
{
    return *Synchro;
}
void setSync(int val)
{
    *Synchro = val;
}

void initSync()
{
    key_t key = ftok("keys/Syncman", 65);
    int Syncid = shmget(key, 4, IPC_CREAT | 0644);
    Synchro = (int *) shmat(Syncid, (void *)0, 0);
}
void destroySync(bool delete)
{
    shmdt(Synchro);
    if (delete)
    {
        key_t key = ftok("keys/Syncman", 65);
        int Syncid = shmget(key, 4, 0444);
        shmctl(Syncid, IPC_RMID, NULL);
    }
}

struct Nodemem
{
    int memorysize,nodenumber;
    bool taken;
    struct Nodemem *left;
    struct Nodemem *right;
};

/**
 * @brief  Initialize the memory tree
 * 
 * @param memavailable  The total memory available
 * @param nodenumber  The number of the node
 * @return struct Nodemem* 
 */
struct Nodemem* InitialiseMemory(int memavailable,int nodenumber)
{
    if (memavailable < 1) { return NULL; }
    struct Nodemem *root = malloc(sizeof(struct Nodemem));
    root->nodenumber = nodenumber;
    root->memorysize = memavailable;
    root->taken = false;
    root->left = InitialiseMemory(memavailable / 2,2*nodenumber);
    root->right = InitialiseMemory(memavailable / 2,2*nodenumber+1);
    return root;
}
/**
 * @brief Set the Taken status of a certain node's children as taken
 * 
 * @param root  The root of the memory tree
 */
void SetChildrenAsTaken(struct Nodemem* root){
    if (root == NULL) { return; }
    root->taken = true;
    SetChildrenAsTaken(root->left);
    SetChildrenAsTaken(root->right);
}

/**
 * @brief Set the Taken status of a certain node's children as free
 * 
 * @param root  The root of the memory tree
 */
void SetChildrenFree(struct Nodemem* root){
    if (root == NULL) { return; }
    root->taken = false;
    SetChildrenFree(root->left);
    SetChildrenFree(root->right);
}

/**
 * @brief  Check if memory is available for a process
 * 
 * @param root  The root of the memory tree
 * @return true 
 * @return false 
 */
bool CheckMemoryAvailability(struct Nodemem* root){
    if (root == NULL) { return false; }
    if (root->taken) { return false; }
    if (root->left) { 
        if (root->left->taken || root->right->taken) { return false; }
        return CheckMemoryAvailability(root->left) && CheckMemoryAvailability(root->right);
    }
    return true;
}

/**
 * @brief  Clear the memory tree
 * 
 * @param root  The root of the memory tree
 */
void ClearMemory(struct Nodemem* root)
{
    if (root == NULL) { return; }
    ClearMemory(root->left);
    ClearMemory(root->right);
    free(root);
}

/**
 * @brief  Allocate memory for a process
 * 
 * @param root The root of the memory tree
 * @param memrequired  The memory required by the process
 * @param p  The process that needs the memory
 * @param totalmemory  The total memory available
 * @return true 
 * @return false 
 */
bool AllocateMemory(struct Nodemem* root, int memrequired,struct process* p,int* totalmemory) {
    if (root == NULL) { return false; }
    if (root->taken || *totalmemory < memrequired) { return false; }
    if (root->memorysize == memrequired) {
        if (root->left) { if (root->left->taken || root->right->taken) { return false; } }
        if (root->taken) { return false; }
        if (CheckMemoryAvailability(root)){
            SetChildrenAsTaken(root);
            char RunningTimeStr[12];
            sprintf(RunningTimeStr, "%d", p->runningtime);
            char *args[3] = {"./process.out", RunningTimeStr, NULL};
            pid_t pid = fork();
            if (pid == 0){ execv(args[0], args);}
            p->pid = pid;
            *totalmemory -= root->memorysize;
            p->mem = root;
            p->memoryused = root->memorysize;
            return true;
        }
        return false;
    }
    if (root->memorysize > memrequired) {
        if (AllocateMemory(root->left, memrequired,p,totalmemory)) { return true; }
        else if (AllocateMemory(root->right, memrequired,p,totalmemory)) { return true; }
        else {
            if (CheckMemoryAvailability(root)){
                SetChildrenAsTaken(root);
                char RunningTimeStr[12];
                sprintf(RunningTimeStr, "%d", p->runningtime);
                char *args[3] = {"./process.out", RunningTimeStr, NULL};
                pid_t pid = fork();
                if (pid == 0){ execv(args[0], args);}
                p->pid = pid;
                p->mem = root;
                p->memoryused = root->memorysize;
                *totalmemory -= root->memorysize;
                return true;
            }
            return false;
        }
    }
    return false;
}

/**
 * @brief Logs the memory allocation and deallocation of a process
 * 
 * @param allocate  A boolean to indicate if the process is being allocated or deallocated
 * @param root  The root of the memory tree
 * @param p  The process that needs the memory
 * @param f  The file pointer to the log file
 */
void MemoryLogger(bool allocate,struct Nodemem* root, struct process* p,FILE* f){
    if (p->mem == NULL) { return; }
    struct Nodemem* node = p->mem;
    int memstart, memend, nodenumber = node->nodenumber;
    int florida = floor(log2(nodenumber));
    memstart = (MAX_SIZE/(pow(2,florida+1) - pow(2,florida))) * (nodenumber - pow(2,florida));
    memend = memstart + node->memorysize - 1;
    if (allocate) { fprintf(f,"At time %d allocated %d bytes for process %d from %d to %d\n",getClk(),p->memsize,p->id,memstart,memend); }
    else { fprintf(f,"At time %d freed %d bytes for process %d from %d to %d\n",getClk(),p->memsize,p->id,memstart,memend); }
}

/**
 * @brief  Deallocate memory for a process
 * 
 * @param p  The process that needs the memory
 * @param totalmemory The total memory available 
 * @return true 
 * @return false 
 */
bool DeAllocateMemory(struct process* p,int* totalmemory){
    if (p->mem->taken){ 
        SetChildrenFree(p->mem);
        *totalmemory += p->mem->memorysize;
        p->memoryused = 0;
        return true;
    }
    else { return false; }
}


#endif // HEADERS_H