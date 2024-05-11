#include "headers.h"
int isRunning = 0;

int main(int argc, char * argv[])
{
    if (argc != 2) {
        perror("Invalid number of arguments");
        return -1;
    }
    int SendQueueID,ReceiveQueueID;
    DefineKeysProcess(&SendQueueID,&ReceiveQueueID);
    initClk();
    int remainingTime = atoi(argv[1]),currentClock=getClk();
    while (remainingTime > 0){
        currentClock = getClk();
        struct msgbuff turn;
        int received=msgrcv(SendQueueID,&turn,sizeof(turn.msg),getpid(),!IPC_NOWAIT);
        if(received != -1){ remainingTime--; }
        struct msgbuff rem;
        rem.mtype=getpid();
        rem.msg=remainingTime;
        int sent=msgsnd(ReceiveQueueID,&rem,sizeof(rem.msg),!IPC_NOWAIT);
        while(currentClock == getClk()){};
    }
    destroyClk(false);
    return 0;
}
