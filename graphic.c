#include <time.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include "dependecies.c"

#define MIN_PROCESS_SLICE 4

#define CLEAR_ALL_SCREEN 2
#define CLEAR_CURSOR_TO_END_SCREEN 0
#define CLEAR_START_SCREEN_TO_CURSOR 1

#define CLEAR_ALL_LINE 2
#define CLEAR_CURSOR_TO_END_LINE 0
#define CLEAR_START_LINE_TO_CURSOR 1

#define BLACK 	40
#define RED 	41
#define GREEN 	42
#define YELLOW 	43
#define BLUE 	44
#define MAGENTA 45
#define CYAN 	46

typedef struct  {
	double sum;
	double segment[MAX_PROC];
} SumAverageTuple;

// ---------------------------------
// ------- DRAWING FUNCTIONS -------
// ---------------------------------
void resetColor();
void loadingDraw();
void resetGraphic();
void drawBrightWhite();
void setColor(int color);
void setBackground(int color);

void setColumn(int col);
void clearLine(int flag);
void clearScreen(int flag);
void setPosition(int row, int col);

void moveCursorUp(int numOfLines);
void moveCursorDown(int numOfLines);
void moveCursorLeft(int numOfLines);
void moveCursorRight(int numOfLines);


// ---------------------------------
// ------------ TOOLS --------------
// ---------------------------------
int kbhit(void);
void delay(float);
void initProgram();
GanttDiag * initRoundRobin(UserList *, float);
GanttDiag * Round_Robin(UserList *user_list,float quantum);

void drawInitialUserData();
void drawGanttDiag(GanttDiag *);
void drawUserData(const struct User * );
void drawInitialUserTable(const UserList *);

SumAverageTuple * getSumOfBurstTime(
	struct Process * const, const unsigned int
);



int main() {
	// loadingDraw();
	initProgram();

	UserList *user_list = createUserList();

	fin = fopen("data.txt","w");
    checkAlloc(fin);

    int nrUsers;
    float quantum;

    // ftok to generate unique key 
	key_t key = ftok("shmfile",65); 

	// shmget returns an identifier in shmid 
	int shmid = shmget(key, 1024, 0666 | IPC_CREAT); 

	// shmat to attach to shared memory 
	current_running_process_pid = (char *) shmat(shmid, (void *) 0, 0);
	// end of acces shared memory 

    generateData(user_list,&nrUsers,&quantum);
   	drawInitialUserTable(user_list);
   	
   	GanttDiag * diag = initRoundRobin(user_list, quantum);
   	drawGanttDiag(diag);
   	
   	//detach from shared memory 
	shmdt(current_running_process_pid); 
	// destroy the shared memory 
	shmctl(shmid,IPC_RMID,NULL);

	free(diag);
	free(user_list);
    fclose(fin);
    return 0;
}

GanttDiag * Round_Robin(UserList *user_list,float quantum)
{
    GanttDiag *diagram = createDiagram();

    fprintf(fin,"\n----------------------Round-Robin--------------------\n");
    clearScreen(2);
    setPosition(0, 0);

    while(userExists(user_list))
    {
        //get the first user
        struct User* user = user_list->first;
        fprintf(fin,"\nUser %d has been selected\n",user->index);
        // printUserProcesses(user);
        delay(0.5);
        clearScreen(2);
    	setPosition(0, 0);
        printf("Running Round-Robin...\n\n");
        printf("User %d has been selected\t",user->index);

        sprintf(current_running_process_pid, "%d", user->nextProcess->proc_pid);
    	
    	
        // while it's current user's turn
        // and there are processes
        float timeSlice = user->timeQuantum;
        while(timeSlice > 0 && user->processCount > 0)
        {
            // get the first process in the queue

            struct Process *process = user->nextProcess;

            // create a diagram node to be added to the diagram
            // after we find the time spend on the CPU

            GanttProc *gproc = createProcDiagram(process->proc_pid,user->index,0);
            
            fprintf(fin,"\nSelected process; burst-time= %f \n\n",process->burstTime);
            
            // if process burst time is greater than quantum

            if(process->burstTime - quantum > 0)
            {
                // put current process on CPU for 'quantum' ms
                // decrease its burst time

                process->burstTime-=quantum;

                //add time spend on the CPU to diagram

                gproc->proc_time = quantum;

                // quantum ms have passed from the total user's time

                timeSlice -=quantum;

                //move to next process in queue

                moveToNextProcess(user);
                
            }
            else
            {
                // the burst time < quantum
                // then put process on CPU for "burst_time" ms

                timeSlice -= process->burstTime;


                //add time spend on the CPU to diagram

                gproc->proc_time = process->burstTime;

                // current process is done, remove it from current user's process queue

                removeCurrentProcessFromUser(user);
                // delay(0.3);
            }
            // add process to the CPU(i.e to the Gantt Diagram)
            addProcToDiagram(diagram, gproc);


        }   // end of timeSlice for current user
        fprintf(fin,"Same user after his timeSlice\n");
        printUserProcesses(user);


        // if the current user does not have any another process for the next turn
        // remove the current user
        if(user->processCount == 0)
        {
            fprintf(fin,"User %d has finished\n",user->index);

             removeCurrentUser(user_list);

        }
        // go to the next user
        else
            user_list->first = user_list->first->next;

    } // end of while(thereAreUsers)

    // return the Gantt Diagram
    return diagram;
}


void delay(float number_of_seconds) { 
    // Converting time into milli_seconds 
    int milli_seconds = CLOCKS_PER_SEC * number_of_seconds; 
  
    // Stroing start time 
    clock_t start_time = clock(); 
  
    // looping till required time is not acheived 
    while (clock() < start_time + milli_seconds); 
}

void loadingDraw() {
	while(1) {
		printf("| Loading\n");
		delay(0.3);
		moveCursorUp(1);
		clearLine(CLEAR_START_LINE_TO_CURSOR);
		printf("\n");
		moveCursorUp(1);
		printf("\\ Loading\n");
		delay(0.3);
		moveCursorUp(1);
		clearLine(CLEAR_START_LINE_TO_CURSOR);
		printf("\n");
		moveCursorUp(1);
		printf("- Loading\n");
		delay(0.3);
		moveCursorUp(1);
		clearLine(CLEAR_START_LINE_TO_CURSOR);
		printf("\n");
		moveCursorUp(1);
		printf("/ Loading\n");
		delay(0.3);
		moveCursorUp(1);
		clearLine(CLEAR_START_LINE_TO_CURSOR);
		printf("\n");
		moveCursorUp(1);

	}
}

GanttDiag * initRoundRobin(UserList * user_list, float quantum) {
	printf("Start Round-Robin (Y/N): ");
    char c;
    while (1) {
    	if (kbhit()) {
	        c = getchar();
	        if(c == 'Y' || c == 'y') {
	            return Round_Robin(user_list, quantum);
                break;
	        } else if (c == 'N' || c == 'n') {
	        	return NULL;
                break;
	        } else {
	        	printf("\nStart Round-Robin (Y/N): ");
	        }
    	}
    }	
}

void drawGanttDiag(GanttDiag * diag) {
	clearScreen(2);
    setPosition(0, 0);

    drawBrightWhite();
    printf("\n\t\t\t\tGantt Diagram\n\n");
   	GanttProc * iterator = diag->first;
   	resetGraphic();
   	if (diag->first == diag->last) {
   		setColor(30);
   		setBackground(46);
   		printf("\tProcess with PID %d terminated with CPU time=%f \t \n", iterator->proc_index,iterator->proc_time);
   		resetGraphic();
   	}

   	int k = 0;
   	while (iterator != diag->last) {
   		setColor(30);
   		if (k++ % 2) setBackground(46);
   		else setBackground(43);
   		printf("\tProcess with PID %d terminated with CPU time=", iterator->proc_index);
        printf("%f", iterator->proc_time);
        if (iterator->proc_time < 10)
            printf(" ");
        printf("\t\t\n"); 
   		iterator = iterator->next;

   		resetGraphic();
   	}
   	printf("\n\n\n");
}

SumAverageTuple * getSumOfBurstTime(struct Process * const processList, const unsigned int numOfProcesses) {
    
	SumAverageTuple * sumAverageTuple = (SumAverageTuple *) malloc(sizeof(SumAverageTuple));
	checkAlloc(sumAverageTuple);

	sumAverageTuple->sum = 0;	
	struct Process * iterator = processList;

	if (!iterator) 
		return 0;

    for (size_t i = 0; i < numOfProcesses; i++) {
        sumAverageTuple->sum += iterator->burstTime;
        sumAverageTuple->segment[i] = iterator->burstTime;
        iterator = iterator->next;
    }

    for (size_t i = 0; i < numOfProcesses; i++) {
        sumAverageTuple->segment[i] = 
        	  sumAverageTuple->segment[i] / 
        				 sumAverageTuple->sum;
    }

    return sumAverageTuple;
}

void drawUserData(const struct User *user) {
	drawBrightWhite();
	
	printf("User ");
	if (user->index < 10) printf(" ");
	
	printf("%d   ->   |\tWeight: %0.4f \t|\tTimeSlice: %.4f\t|\tNumProcess: %d\t\t|\n", 
		user->index, user->weight, user->timeQuantum, user->processCount);
	// printUserProcesses(user);
	
	SumAverageTuple * total = getSumOfBurstTime(user->nextProcess, user->processCount);
	// double sum = 0;
	// printf("%f\n", total->sum);
	for (int i = 0; i < user->processCount; i++) {
		total->segment[i] = (int)(total->segment[i] * 30);
		// printf("%f ", total->segment[i]);
		// sum += total->segment[i];
	}
	// printf(":  %f\n", sum);

	int new_color;
	int current_color = rand() % 6 + 41;
	resetColor();
	setColor(33);
	printf("Process queue: ");
	setColor(37);

	struct Process * iterator = user->nextProcess;
	int index = 0, i = 0;
	for (int index = 0; index < user->processCount; index++) {
		resetGraphic();
		setBackground(current_color);

		int middle = total->segment[index] / 2;
		while (total->segment[index]) {
			if (--total->segment[index] == middle) {
				printf(" P%d", iterator->proc_pid);
			} else {
				printf("   ");
			}
			i++;
		}


		do {
			new_color = rand() % 6 + 41;
		} while (new_color == current_color);
		current_color = new_color; 
		iterator = iterator->next;
	}
	
	while (i++ < 30) printf("   ");
	resetGraphic(); 

	free(total);

}

void drawInitialUserTable(const UserList * user_list) {
	delay(2);
	struct User * start = user_list->first;
   	struct User * iterator = start;
   	do  {
   		printf("\n");
    	drawUserData(iterator);
    	iterator = iterator->next;
    	printf("\n");
	} while (iterator != start);

    resetGraphic();
    printf("\n");
    printf("\n");
}

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    
    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    
    return 0;
}

void initProgram() {
	srand(time(NULL));
	clearScreen(2);
	setPosition(0, 0);
}

void resetGraphic() 
{ printf("\033[0m"); }

void drawBrightWhite()
{ printf("\033[37;1m"); }

void resetColor() 
{ printf("\033[0m"); }

void setColor(int color)
{ printf("\033[%dm", color); }

void setBackground(int color)
{ printf("\033[%d;1m", color); }

void clearLine(int flag) 
{ printf("\033[%dK", flag); }

void clearScreen(int flag)
{ printf("\033[%dJ", flag); }

void setColumn(int col) 
{ printf("\033[%dG", col); }

void setPosition(int row, int col) 
{ printf("\033[%d;%dH", row, col); }

void moveCursorUp(int numOfLines) 
{ printf("\033[%dA", numOfLines); }

void moveCursorDown(int numOfLines) 
{ printf("\033[%dB", numOfLines); }

void moveCursorRight(int numOfLines) 
{ printf("\033[%dC", numOfLines); }

void moveCursorLeft(int numOfLines)
{ printf("\033[%dD", numOfLines); }








