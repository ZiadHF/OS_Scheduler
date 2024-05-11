#include <errno.h>
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#define GUI_PROPERTY_LIST_IMPLEMENTATION
#include "raygui.h"
#include "gui_window_file_dialog.h"
#include "style_cyber.h"
#include "style_bluish.h"
#include <stdio.h>
#include <time.h>
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
#include <string.h>
int pageShifter = 0;
int *clkptr;
int clkid;
int clkFound = 0;
float cpu_utilization = 0.0f, avg_wta = 0.0f, avg_waiting = 0.0f, std_wta = 0.0f;
float memoryutilization = 0.0f,memorytaken = 0.0f;
struct Nodemem
{
    int memorysize;
    bool taken,isLeft;
    struct Nodemem *left;
    struct Nodemem *right;
};


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
    struct Nodemem* mem;
};
int isInteger(const char *str)
{
    int i = 0;
    while (str[i] == ' ' || str[i] == '\t')
    {
        i++;
    }
    while (str[i] >= '0' && str[i] <= '9')
    {
        i++;
    }
    return str[i] == '\0';
}
void clearCharArray(char *array, int length)
{
    memset(array, '\0', length);
}

bool isProcessExist(int id, int numProcesses, struct process rdyProcList[])
{
    for (int i = 0; i < numProcesses; i++)
    {
        if (rdyProcList[i].id == id)
        {
            return true;
        }
    }
    return false;
}
void sigint_handler(int sig)
{
    pageShifter = 3;
    FILE *file = fopen("scheduler.perf", "r");

    if (file == NULL)
    {
        printf("Error opening the file.\n");
        return;
    }
    fscanf(file, "CPU Utilization = %f %%\n", &cpu_utilization);
    fscanf(file, "Avg WTA = %f\n", &avg_wta);
    fscanf(file, "Avg Waiting = %f\n", &avg_waiting);
    fscanf(file, "Std WTA = %f\n", &std_wta);
    fclose(file);
}

void addProcess(struct process newProcess, int *numProcesses, struct process rdyProcList[])
{
    if (!isProcessExist(newProcess.id, *numProcesses, rdyProcList))
    {
        rdyProcList[(*numProcesses)] = newProcess;
        *numProcesses = (*numProcesses) + 1;
    }
}
void removeProcessByID(int idToRemove, int *numProcesses, struct process rdyProcList[])
{
    int i, found = 0;
    for (i = 0; i < *numProcesses; i++)
    {
        if (rdyProcList[i].id == idToRemove)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {

        for (; i < (*numProcesses) - 1; i++)
        {
            rdyProcList[i] = rdyProcList[i + 1];
        }
        (*numProcesses)--;
    }
}

int main(void)
{
    signal(SIGINT, sigint_handler);
    int ss = 0;
    const int screenWidth = 1280;
    const int screenHeight = 800;
    int loop = 1;
    InitWindow(screenWidth, screenHeight, "OS Project");
    /* Title Page Variables */
    bool lightDarkBool = false;                // light/dark mode boolean
    char *lightDarkMode = "Toggle Light Mode"; // light/dark mode label text
    GuiLoadStyleCyber();                       // init state is dark
    int endOfAnimWidth = 0;                    // BG animation width
    float time_wave = 0;                       // moves the wave

    /*Choose algorithm Page Variables*/
    bool dropdownAlgo = false;
    int algoChoice = 0;
    char filePath[1025];
    char fileText[1025];
    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    bool rrError = false;
    bool txtError = false;
    bool testgenError = false;
    bool genfile = false;
    char rrQuantum[100] = "";
    char ProcessCount[100] = "";
    /*Simulation Page Variables*/
    int listViewScrollIndex = 0;
    int listViewActive = -1;
    int listViewScrollIndex2 = 0;
    int listViewActive2 = -1;
    int listViewScrollIndex3 = 0;
    int listViewActive3 = -1;
    int listViewScrollIndex4 = 0;
    int listViewActive4 = -1;
    char rdyList[5000] = "";
    char arrivedList[5000] = "";
    pid_t ProcessGen;
    char doneList[5000] = "";
    char workingList[1024] = "n/a";
    char memList[5000] = "";

    /*  Page List:
        0 -> Title Page
        1 -> Choose Schduler Page
        2 -> Simulation Page
        3 -> Statistics Page
    */

    float animationSpeed = 2.0f;
    SetTargetFPS(60);
    Vector2 pointsSine[screenWidth];
    Vector2 pointsCosine[screenWidth];
    struct process doneProcList[3000];
    struct process rdyProcList[3000];
    struct process arrivedProcList[3000];
    int deadindex = 0;
    int rdyindex = 0;
    int arrivedindex = 0;
    key_t GUIKey,ArrivedProcessesKey;
    int GUIID,ArrivedProcessesID;
    GUIKey = ftok("keys/Guiman", 'A');
    ArrivedProcessesKey = ftok("keys/Guiman", 'B');
    GUIID = msgget(GUIKey, 0666 | IPC_CREAT);
    printf("%i\n\n\n\n", GUIID);
    if (GUIID == -1)
    {
        perror("Error in create message queue");
        exit(-1);
    }
    ArrivedProcessesID = msgget(ArrivedProcessesKey, 0666 | IPC_CREAT);
    if (ArrivedProcessesID == -1)
    {
        perror("Error in create message queue");
        exit(-1);
    }
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
    struct process ArrivedProcess;
    while (loop)
    {
        if (ss == 1)
        {
            TakeScreenshot("schedulerperfimage.png");
            ss = 3;
        }
        int received = msgrcv(GUIID, &ArrivedProcess, sizeof(ArrivedProcess), 0, IPC_NOWAIT);
        if (received != -1)
        {
            addProcess(ArrivedProcess, &rdyindex, rdyProcList);
            clearCharArray(rdyList, 5000);
            clearCharArray(memList, 5000);
            for (int i = 0; i < rdyindex; i++)
            {
                char temp[1000];
                if (rdyProcList[i].mem != NULL){
                    sprintf(temp, "P%d - %d bytes;", rdyProcList[i].id,rdyProcList[i].memsize);
                    strcat(memList, temp);
                }
                if (rdyProcList[i].id == *runningProcess)
                {
                    continue;
                }
                sprintf(temp, "P%d;", rdyProcList[i].id);
                strcat(rdyList, temp);
            }
        }
        received = msgrcv(ArrivedProcessesID, &ArrivedProcess, sizeof(ArrivedProcess), 0, IPC_NOWAIT);
        if (received != -1)
        {
            addProcess(ArrivedProcess, &arrivedindex, arrivedProcList);
            clearCharArray(arrivedList,5000);
            for (int i = 0; i < arrivedindex; i++)
            {
                char temp[1000];
                sprintf(temp, "P%d;", arrivedProcList[i].id);
                strcat(arrivedList, temp);
            }
        }

        if (*runningProcess != -1)
        {
            sprintf(workingList, "P%i", *runningProcess);

            clearCharArray(rdyList, 5000);
            clearCharArray(memList, 5000);
            clearCharArray(arrivedList,5000);
            for (int i = 0 ; i < arrivedindex ; i++)
            {
                char temp[1000];
                sprintf(temp, "P%d;", arrivedProcList[i].id);
                strcat(arrivedList, temp);
            }
            memoryutilization = 0, memorytaken = 0;
            for (int i = 0; i < rdyindex; i++)
            {
                char temp[1000];
                if (rdyProcList[i].mem != NULL){
                    sprintf(temp, "P%d - %d byte(s) - %d byte(s);", rdyProcList[i].id,rdyProcList[i].memsize,rdyProcList[i].memoryused);
                    strcat(memList, temp);
                }
                if (rdyProcList[i].id == *runningProcess)
                {
                    continue;
                }
                sprintf(temp, "P%d;", rdyProcList[i].id);
                strcat(rdyList, temp);
            }
        }
        if (*deadProcess != -1)
        {
            if (*runningProcess == *deadProcess)
            {
                sprintf(workingList, "n/a");
            }
            struct process x;
            x.id = *deadProcess;
            addProcess(x, &deadindex, doneProcList);
            removeProcessByID(x.id, &rdyindex, rdyProcList);
            removeProcessByID(x.id, &arrivedindex, arrivedProcList);
            clearCharArray(doneList, 5000);
            for (int i = 0; i < deadindex; i++)
            {
                char temp[1000];
                sprintf(temp, "P%d;", doneProcList[i].id);
                strcat(doneList, temp);
            }
        }
        for (int i = 0 ; i < arrivedindex ; i++){
            for (int j = 0 ; j < rdyindex ; j++){
                if (arrivedProcList[i].id == rdyProcList[j].id){ removeProcessByID(arrivedProcList[i].id,&arrivedindex,arrivedProcList);}
            }
        }
        for (int i = 0 ; i < rdyindex ; i++){
            memoryutilization += (float)rdyProcList[i].memsize;
            memorytaken += (float)rdyProcList[i].memoryused;
        }
        memoryutilization = memoryutilization/1024 * 100;
        memorytaken = memorytaken/1024 * 100;
        BeginDrawing();
        DrawFPS(0, 0);
        for (int x = 0; x < endOfAnimWidth; x++)
        {
            float normalizedX = x / (float)screenWidth;
            float angle = PI * 2 * normalizedX + time_wave;
            float y = sin(angle) * (screenHeight / 4);
            pointsSine[x] = (Vector2){x, screenHeight / 2 - (int)y};
            float y2 = cos(angle + PI / 2) * (screenHeight / 4);
            pointsCosine[x] = (Vector2){x, screenHeight / 2 - (int)y2};
            if (x == endOfAnimWidth - 1)
            {
                DrawCircle(x, screenHeight / 2 - (int)y, 20, RED);
                DrawCircle(x, screenHeight / 2 - (int)y2, 20, BLUE);
            }
        }
        DrawLineStrip(pointsSine, endOfAnimWidth, RED);
        DrawLineStrip(pointsCosine, endOfAnimWidth, BLUE);
        float frameTime = GetFrameTime();
        time_wave += frameTime * animationSpeed;
        if (endOfAnimWidth < screenWidth - 50)
        {
            endOfAnimWidth += 6;
        }
        if (pageShifter == 0)
        {
            GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            /* Top Right buttons*/
            if (GuiButton((Rectangle){screenWidth - 120, 30, 100, 40}, "Close"))
            {
                loop--;
            }
            if (GuiButton((Rectangle){screenWidth - 320, 30, 180, 40}, lightDarkMode))
            {
                lightDarkBool = !lightDarkBool;
                lightDarkMode = (lightDarkBool ? "Toggle Dark Mode" : "Toggle Light Mode");
                lightDarkBool ? GuiLoadStyleBluish() : GuiLoadStyleCyber();
            }
            /*Title label*/
            GuiSetStyle(DEFAULT, TEXT_SIZE, 120);
            GuiLabel((Rectangle){30, screenHeight / 2 - (120), screenWidth, 120}, "Scheduler");
            GuiLabel((Rectangle){30, screenHeight / 2 - (120) + 100, screenWidth, 120}, "Project");
            GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
            if (GuiButton((Rectangle){30, screenHeight / 2 - (120) + 240, 100, 40}, "Start"))
            {
                pageShifter = 1;
            }
        }

        if (pageShifter == 1)
        {
            if (fileDialogState.windowActive)
            {
                GuiLock();
            }
            GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            /* Top buttons*/
            if (GuiButton((Rectangle){screenWidth - 120, 30, 100, 40}, "Close"))
            {
                loop--;
            }
            if (GuiButton((Rectangle){screenWidth - 320, 30, 180, 40}, lightDarkMode))
            {
                lightDarkBool = !lightDarkBool;
                lightDarkMode = (lightDarkBool ? "Toggle Dark Mode" : "Toggle Light Mode");
                lightDarkBool ? GuiLoadStyleBluish() : GuiLoadStyleCyber();
            }
            if (GuiButton((Rectangle){30, 30, 100, 40}, "Back"))
            {
                pageShifter = 0;
            }
            /*Main Content*/
            GuiSetStyle(DEFAULT, TEXT_SIZE, 80);
            GuiLabel((Rectangle){30, 50, screenWidth, 120}, "Choose a algorithm:");
            GuiSetStyle(DEFAULT, TEXT_SIZE, 30);

            if (GuiDropdownBox((Rectangle){30, 180, 600, 60}, "Round Robin;Shortest Remaining Time Next;Highest Priority First", &algoChoice, dropdownAlgo))
            {
                dropdownAlgo = !dropdownAlgo;
            }
            if (algoChoice == 0)
            {
                Rectangle rrRect = {730, 180, 500, 60};
                GuiLabel((Rectangle){730, 130, 500, 60}, "Enter Quantum:");
                GuiTextBox(rrRect, rrQuantum, 20, (CheckCollisionPointRec(GetMousePosition(), rrRect) ? true : false));
                if (rrError)
                {
                    DrawText("Please enter a valid quantum.", 730, 280, 30, RED);
                }
            }
            if (GuiCheckBox((Rectangle){30, 300, 20, 20}, "Test Generator", &genfile))
            {
            }
            if (genfile)
            {
                Rectangle NumRect = {30, 400, 500, 60};
                GuiLabel((Rectangle){30, 350, 500, 60}, "Enter Number of Processes:");
                GuiTextBox(NumRect, ProcessCount, 20, (CheckCollisionPointRec(GetMousePosition(), NumRect) ? true : false));
                if (testgenError)
                {
                    DrawText("Please enter a valid number of processes.", 30, 500, 30, RED);
                }
            }
            GuiSetStyle(DEFAULT, TEXT_SIZE, 80);
            strcpy(fileText, "File :");
            strcat(fileText, fileDialogState.dirPathText);
            strcat(fileText, "/");
            strcat(fileText, fileDialogState.fileNameText);
            GuiLabel((Rectangle){30, 500, screenWidth, 120}, "Choose your process text file:");

            GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
            if (txtError)
            {
                DrawText("Input file must be a txt file.", 30, 600, 30, RED);
            }
            GuiLabel((Rectangle){30, 700, screenWidth, 120}, fileText);
            if (GuiButton((Rectangle){30, 650, 140, 30}, GuiIconText(ICON_FILE_OPEN, "Open File")))
            {
                fileDialogState.windowActive = true;
            }
            if (GuiButton((Rectangle){screenWidth - 120, screenHeight - 50, 100, 40}, "Simulate"))
            {
                txtError = false;
                rrError = false;
                testgenError = false;
                if (strstr(fileDialogState.fileNameText, ".txt") == NULL)
                {
                    txtError = true;
                }
                if (algoChoice == 0)
                {
                    if (!isInteger(rrQuantum) || rrQuantum[0] == '\0')
                    {
                        rrError = true;
                    }
                }
                if (!isInteger(ProcessCount) || ProcessCount[0] == '\0')
                {
                    testgenError = true;
                }
                if (!txtError && !rrError && !genfile)
                {
                    char algoChoicestr[5];
                    sprintf(algoChoicestr, "%d", algoChoice + 1);
                    char *args[] = {"./process_generator.out", fileDialogState.fileNameText, algoChoicestr, rrQuantum, NULL};
                    ProcessGen = vfork();
                    if (ProcessGen == 0)
                    {
                        execv(args[0], args);
                    }
                    pageShifter = 2;
                }
                if (!rrError && !testgenError && genfile)
                {
                    char *args_test[] = {"./test_generator.out", ProcessCount, NULL};
                    pid_t TestGen = vfork();
                    if (TestGen == 0)
                    {
                        execv(args_test[0], args_test);
                    }

                    waitpid(TestGen, NULL, 0);

                    char algoChoicestr[5];
                    sprintf(algoChoicestr, "%d", algoChoice + 1);
                    char *args[] = {"./process_generator.out", "processes.txt", algoChoicestr, rrQuantum, NULL};
                    ProcessGen = vfork();
                    if (ProcessGen == 0)
                    {
                        execv(args[0], args);
                    }
                    pageShifter = 2;
                }
            }

            GuiUnlock();
            GuiWindowFileDialog(&fileDialogState);
        }
        if (pageShifter == 2)
        {
            if (clkFound == 0)
            {
                clkid = shmget(300, 4, 0444);
                while ((int)clkid == -1)
                {
                    printf("Wait! The clock not initialized yet!\n");
                    sleep(1);
                    clkid = shmget(300, 4, 0444);
                }
                clkptr = (int *)shmat(clkid, (void *)0, 0);
                clkFound = 1;
            }
            GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
            char timetemp[1000];
            sprintf(timetemp, "Current CLK : %i", *clkptr);
            GuiLabel((Rectangle){120, 100, 600, 30}, timetemp);
            GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
            sprintf(timetemp, "Memory Taken : %.2f%%", memorytaken);
            GuiLabel((Rectangle){screenWidth - 550, 100, 600, 30}, timetemp);
            sprintf(timetemp, "Memory Utilization : %.2f%%", memoryutilization);
            GuiLabel((Rectangle){screenWidth - 300, 100, 600, 30}, timetemp);
            GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
            /* Top buttons*/
            if (GuiButton((Rectangle){screenWidth - 120, 30, 100, 40}, "Close"))
            {
                loop--;
            }
            if (GuiButton((Rectangle){screenWidth - 320, 30, 180, 40}, lightDarkMode))
            {
                lightDarkBool = !lightDarkBool;
                lightDarkMode = (lightDarkBool ? "Toggle Dark Mode" : "Toggle Light Mode");
                lightDarkBool ? GuiLoadStyleBluish() : GuiLoadStyleCyber();
            }
            GuiGroupBox((Rectangle){120, 150, 400, 200}, "Working Process");
            DrawText(workingList, 310, 235, 20, PURPLE);
            GuiGroupBox((Rectangle){120, 400, 400, 300}, "Done Processes");
            GuiListView((Rectangle){120, 420, 400, 300}, doneList, &listViewScrollIndex, &listViewActive);
            GuiGroupBox((Rectangle){screenWidth - 550, 150, 200, 300}, "RDY Queue");
            GuiListView((Rectangle){screenWidth - 550, 150 + 20, 200, 300}, rdyList, &listViewScrollIndex2, &listViewActive2);
            GuiGroupBox((Rectangle){screenWidth - 300, 150, 200, 300}, "WTN Queue");
            GuiListView((Rectangle){screenWidth - 300, 150 + 20, 200, 300}, arrivedList, &listViewScrollIndex4, &listViewActive4);
            GuiGroupBox((Rectangle){screenWidth - 550, 500, 450, 200}, "MEM Used");
            GuiListView((Rectangle){screenWidth - 550, 500 + 20, 450, 200}, memList, &listViewScrollIndex3, &listViewActive3);
            //DrawText("MEM WIP", screenWidth - 550 + 150, 590, 20, RED);
        }
        if (pageShifter == 3)
        {
            GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
            /* Top buttons*/
            if (GuiButton((Rectangle){screenWidth - 120, 30, 100, 40}, "Close"))
            {
                loop--;
            }
            if (GuiButton((Rectangle){screenWidth - 320, 30, 180, 40}, lightDarkMode))
            {
                lightDarkBool = !lightDarkBool;
                lightDarkMode = (lightDarkBool ? "Toggle Dark Mode" : "Toggle Light Mode");
                lightDarkBool ? GuiLoadStyleBluish() : GuiLoadStyleCyber();
            }
            GuiGroupBox((Rectangle){screenWidth / 2 - 200, screenHeight / 2 - 100, 400, 270}, "Performance");
            char fileTemp[300];
            sprintf(fileTemp, "CPU utilization = %.2f %%", cpu_utilization);
            DrawText(fileTemp, screenWidth / 2 - 200 + 50, screenHeight / 2 - 100 + 50, 20, LIGHTGRAY);
            sprintf(fileTemp, "Avg WTA = %.2f ", avg_wta);
            DrawText(fileTemp, screenWidth / 2 - 200 + 50, screenHeight / 2 - 100 + 100, 20, LIGHTGRAY);
            sprintf(fileTemp, "Avg Waiting = %.2f", avg_waiting);
            DrawText(fileTemp, screenWidth / 2 - 200 + 50, screenHeight / 2 - 100 + 150, 20, LIGHTGRAY);
            sprintf(fileTemp, "Std WTA = %.2f", std_wta);
            DrawText(fileTemp, screenWidth / 2 - 200 + 50, screenHeight / 2 - 100 + 200, 20, LIGHTGRAY);
            if (ss == 0)
            {
                ss = 1;
            }
        }
        EndDrawing();
    }
    shmctl(runningID, IPC_RMID, NULL);
    shmctl(deadID, IPC_RMID, NULL);
    shmctl(clkid, IPC_RMID, NULL);
    kill(getpgrp(), SIGINT);
    CloseWindow();
    return 0;
}
