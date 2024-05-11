#include "headers.h"
#include "RoundRobin.h"
#include "SRTN.h"
#include "HPF.h"

int main(int argc, char *argv[])
{
    printf("Scheduler\n");
    if (argc != 3)
    {
        perror("Invalid number of arguments");
        return -1;
    }
    initClk();
    printf("Clock initialized in scheduler\n");
    int quantum = atoi(argv[2]), processCount = atoi(argv[1]), SelectedAlgorithm = atoi(argv[0]);
    switch (SelectedAlgorithm){
        case 1:
            RoundRobin(quantum, processCount);
            break;
        case 2:
            SRTN(processCount);
            break;
        case 3:
            HPF(processCount);
            break;
        default:
            perror("Invalid Scheduling Algorithm");
            break;
    }

    // upon termination release the clock resources.
    printf("SCHEDULER DONE\n");
    destroyClk(false);
}