#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <string.h>

/*
    ALL UNITS OF TIME ARE EXPRESSED IN MILLISECONDS
        1 SECOND = 1,000 MILLISECONDS
*/

/*
    TIME_QUANTUM
        -dictates how long a process can stay on the CPU before getting moved back to the queue
        -80% of CPU bursts should be shorter than the time quantum
        -general values: 10 - 100ms
*/

#define TIME_SLICE 10
#define MAX_QUANTUM 15
#define MAX_USERS 5
#define MAX_PROC 3
#define MAX_BURST 40

/*
    struct User
        -weight:       given, weight of user, used to determine the individual time quantum
        -timeQuantum:  generated, obtained by multiplying the user weight with the TIME_QUANTUM
        -processCount: generated, number of active processes
        -nextProcess:  generated, next process that is in queue to get on the CPU
*/



FILE *fin;
struct User{
    float weight;

    float timeQuantum;
    int processCount;
    int index;
    struct Process *nextProcess;
    struct User *next;
    struct User *prev;
};

// userList pointing to the first element of the list

typedef struct{
    struct User *first;

} UserList;


/*
    struct Process
        -burstTime: given, time it takes for the process to complete
        -next:      generated, next process in queue
        -previous:  generated, process before self in queue
*/

struct Process{
    double burstTime;
    pid_t proc_pid;
    struct Process *next;
    struct Process *previous;
};
/*
    Simply linked list used to print the order of processes on CPU

    -proc_index,user_index : used to print the process in the diagram
                             as "P + proc_index + U + user_index"
    -proc_time: time spent by proc_name on the CPU

*/
typedef struct GanttProc{
    double proc_time;
    int proc_index;
    int user_index;
    struct GanttProc *next;
}GanttProc;

// Gantt Diagram List

typedef struct {
    GanttProc * last,*first;
}GanttDiag;

// shared memory
char * current_running_process_pid;

/*
    void checkAlloc()
        -checks if the given pointer has been allocated successfully and exits if it did not
*/

void checkAlloc(void* ptr)
{
    if (!ptr)
    {
        printf("Allocation error!\n");
        exit(1);
    }
}


// creates an empty Gantt Diagram
GanttDiag* createDiagram()
{
    GanttDiag *diagram = (GanttDiag *)malloc(sizeof(GanttDiag));
    checkAlloc(diagram);
    diagram->last = diagram->first = NULL;
    return diagram;

}

// creates new Gantt Diagram node -> a process on the CPU with its index and time spend there
GanttProc *createProcDiagram(int proc_index, int user_index,double proc_time)
{

    GanttProc *p = (GanttProc *)malloc(sizeof(GanttProc));
    checkAlloc(p);

    p->proc_index = proc_index;
    p->proc_time = proc_time;
    p->user_index = user_index;
    p->next = NULL;

    return p;
}

void addProcToDiagram(GanttDiag *diag,GanttProc *p)
{
    // the diagram is empty

    if(diag->last == NULL)
        diag->last = p,diag->first = p;
    else
    {
        // add the process at the end of the diagram
        diag->last->next = p;
        diag->last = p;

    }

}
void printDiagram(GanttDiag *diag)
{
    fprintf(fin,"\n----------Gantt Diagram----------\n");
    for(GanttProc *gproc = diag->first; gproc!=NULL; gproc= gproc->next)
        fprintf(fin,"(P%dU%d): Time on CPU=%f\n",gproc->proc_index,gproc->user_index,gproc->proc_time);
}
/*
    struct User* createNewUser(float userWeight)
        -creates a pointer to a User struct
        -generates the user timeQuantum using the parameter userWeight
*/

struct User* createNewUser(float userWeight,int index)
{
    struct User* newUser = NULL;

    newUser = (struct User*) malloc(sizeof(struct User));
    checkAlloc(newUser);

    newUser->weight = userWeight;
    newUser->timeQuantum = userWeight * TIME_SLICE;
    newUser->processCount = 0;
    newUser->index = index;
    newUser->nextProcess = NULL;
    newUser->next = NULL;
    newUser->prev = NULL;

    return newUser;
}

//creates a circular list of users

UserList *createUserList()
{
    UserList *user_list = (UserList *)malloc(sizeof(UserList));
    checkAlloc(user_list);
    user_list->first = NULL;
    return user_list;
}

// adds an user with weight = userWeight to UserList

struct User *addUser(UserList *user_list,float userWeight,int index)
{
    struct User *u = createNewUser(userWeight,index);

    checkAlloc(u);

    if(user_list->first == NULL)
    {
        user_list->first = u;
        u->next = u;
        u->prev = u;
        return u;
    }


    user_list->first->prev->next = u;
    u->prev = user_list->first->prev;
    u->next = user_list->first;
    user_list->first->prev = u;

    return u;
}

// removes an user from the UserList

void removeCurrentUser(UserList *user_list)
{

    struct User* u = user_list->first;
    user_list->first = user_list->first->next;

    // if there is only one user to delete
    if( u == u->next)
    {
            user_list->first = NULL;
    }
    else{

        u->prev->next = u->next;
        u->next->prev = u->prev;
    }

    free(u);

}
/*
    struct Process* createNewProcess(int processBurstTime)
        -creates a pointer to a Process struct
*/

struct Process* createNewProcess(float processBurstTime,pid_t pid)
{
    struct Process* newProcess = NULL;

    newProcess = (struct Process*) malloc(sizeof(struct Process));
    checkAlloc(newProcess);

    newProcess->burstTime = processBurstTime;
    newProcess->next = NULL;
    newProcess->proc_pid = pid;
    newProcess->previous = NULL;

    return newProcess;
}


/*
    void printUserProcesses(struct User* user)
        -prints user information
        -debug purposes
*/

void printUserProcesses(struct User* user)
{
    struct Process* nextProcess = NULL;
    int i;

    fprintf(fin,"No. of processes %i: ->(Proc. num) : burst-time\n", user->processCount);
    printf("\nNo. of processes %i:\n\n(Proc. num) -> burst-time\n", user->processCount);
    nextProcess = user->nextProcess;

    for (i = 0; i < user->processCount; i++)
    {
        fprintf(fin,"(P%d): %f; ", nextProcess->proc_pid, nextProcess->burstTime);
        printf("(PID: %d) -> %f\n", nextProcess->proc_pid, nextProcess->burstTime);
        nextProcess = nextProcess->next;
    }

    fprintf(fin,"\n");
    printf("\n");
}


// prints the essential data about each user

void printUsers(UserList *user_list)
{

    struct User *u = user_list->first;
    if(user_list->first == NULL)
        return;
    do
    {
        fprintf(fin,"User %d, weight=%f, timeSlice=%f\n",u->index,u->weight,u->timeQuantum);
        printUserProcesses(u);
        fprintf(fin,"\n");
        u=u->next;

    }while(u!=user_list->first);

}


/*
    void linkProcessToUser(struct Process* process, struct User* user)
        -links a process to the end of a user's queue
*/

void linkProcessToUser(struct Process* process, struct User* user)
{
    //The user has no processes, the new one will have to link to itself
    if (user->processCount == 0)
    {
        user->nextProcess = process;
        process->next = process;
        process->previous = process;
    }
    else //The user already has processes, link the chain
    {
        user->nextProcess->previous->next = process;
        process->previous = user->nextProcess->previous;

        process->next = user->nextProcess;
        user->nextProcess->previous = process;
    }

    //Increment the process count
    user->processCount++;
}


/*
    void removeLastProcessFromUser(struct User* user)
        -unlinks and frees the last process of a User
*/
void removeCurrentProcessFromUser(struct User* user)
{
    //Has no processes, nothing to remove
    if (user->processCount == 0)
        return;

    //Process to be removed
    struct Process* removeProcess = NULL;
    removeProcess = user->nextProcess;

    if (kill(removeProcess->proc_pid, SIGTERM) < 0) {
    	printf("Didn't kill process\n");
    }

    user->nextProcess = user->nextProcess->next;

    //Is the only process the User has
    if (user->nextProcess == removeProcess)
    {
        user->nextProcess = NULL;
    }
    else //Other processes exist, link them
    {
        removeProcess->previous->next = removeProcess->next;
        removeProcess->next->previous = removeProcess->previous;
    }

    //Free the memory
    free(removeProcess);

    //Decrease the process count
    user->processCount--;
}


/*
    struct Process* moveToNextProcess(struct User* user)
        -changes the nextProcess pointer of the User
        -returns the process that is now first of queue
*/

struct Process* moveToNextProcess(struct User* user)
{
    //User has no processes, nothing to move
    if (user->processCount == 0)
        return NULL;

    //Get the next process in queue
    struct Process* nextProcess = NULL;
    nextProcess = user->nextProcess->next;

    //Update the nextProcess pointer of User
    user->nextProcess = nextProcess;

    // update running process from shared memory
    sprintf(current_running_process_pid, "%d", nextProcess->proc_pid);

    return nextProcess;
}

void generateData(UserList *user_list,int *nrUsers, float *quantum)
{
    char *argv[] = {"dummy",NULL};

    srand(time(NULL));
    *nrUsers = rand() % MAX_USERS + 1;

    // quantum in interval [10,100]
    float scale = rand()/(float)RAND_MAX;

    *quantum = scale*(MAX_QUANTUM -10) + 10;

    for(int i = 0 ; i< (*nrUsers);i++)
    {
        // generate number between 0.1 and 1
        float weight = ((float)rand()/(float)(RAND_MAX)) +0.1;

        //add user to UserList
        struct User* u = addUser(user_list,weight,i);

        //no. of processes for user "i"

        int nrProc = rand()%MAX_PROC +1;

        //create nrProc processes with a rand. burst time
        // add it to the current user

        for(int j = 0;j<nrProc;j++)
		{
            float burst_time = (float)rand()/(float)(RAND_MAX)*MAX_BURST +0.1;

            pid_t pid = fork();

            if( pid == 0) {
                execve("./dum",argv,NULL);
            }

            struct Process *p = createNewProcess(burst_time,pid);
            linkProcessToUser(p,u);
        }

    }

}

int userExists(UserList *user_list)
{

    return user_list->first != NULL;
}
