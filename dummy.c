// compile this file as "gcc dummy -o dum"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h> 

int main()
{
	// ftok to generate unique key 
	key_t key = ftok("shmfile",65); 

	// shmget returns an identifier in shmid 
	int shmid = shmget(key,1024,0666|IPC_CREAT); 

	// shmat to attach to shared memory 
	char * current_running_process_pid = (char *) shmat(shmid,(void*)0,0); 
	
	char * current_process_pid = (char *) malloc(10 * sizeof(char));
	sprintf(current_process_pid, "%d", getpid());

	while (1) {
		while (!strcmp(current_running_process_pid, current_process_pid));

		while (strcmp(current_running_process_pid, current_process_pid)) {
			/* code to run on process */
			continue;
		}
	}

	free(current_process_pid);
	shmdt(current_running_process_pid);

	return 0;
}