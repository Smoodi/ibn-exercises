/*
 Uni Heidelberg IBN worksheet 2 - exercise 5
 Author: Robin-Marcel Hanne
*/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <string.h>

#define SHM_SIZE 1024 //1KB shared memory

int shmid;

int main() {

 /*
  * Note: Nowadays mmap() is a more modern and adequate way of passing data between programs / threads.
  */

	pid_t _pid; //Process id;
	key_t key;
	char *data;

	int mode;
	
	//We create a shared memory key
	key = 5505; //Our key id
	
	//Creating the segment
	if ((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) {
		perror("Creating shared segment failed.");
		exit(1);
	}
	
	/*
	 * Attaching segment to thread memory
	 */
	 data = shmat(shmid, NULL, 0);
	 if (data == (char *)(-1)) {
		 perror("Data attachment failed");
		 exit(1);
	 }
	 
	 //Writing in the memory.
	 sprintf(data, "%d", -1);
	 
	 //strncpy(data, sprintf(data,-1), SHM_SIZE);
	 
	 //Detaching
	 if (shmdt(data) == -1) {
		 perror("Failed detaching");
		 exit(1);
	 }
	 
	 _pid = fork();
	 if ( _pid == -1 ) {
		 perror("Thread creation failed");
		 exit(1);
	 }
	 if ( _pid == 0 ) {
		 //Child thread
		 int _myId = getpid();
		 
		 printf("We are running the child thread with actual id: %d \n", _myId);
		 
		 //We wait until our memory is not -1;
		 char * _data = shmat(shmid, NULL, 0);
		 if (_data == (char *)(-1)) {
			perror("Data attachment failed");
			exit(1);
		 }
		 
		 //We read the content:
		 printf("We check our data in the child thread: %s \n", _data);
		 
		 int _sharedPID = atoi(_data);
		 
		 //We gotta detach the memory.
		 if (shmdt(_data) == -1) {
			 perror("Failed detaching");
		     exit(1);
		 }
		 
		 if(_sharedPID == -1) {
			 printf("Our parent thread did not write in the data yet. We go sleep again. \n");
			 sleep(1); //We sleep a second.
		 } else {
			 printf("According to main thread we are supposed to be: %d \n", _sharedPID);
			 
			//We talk back to our main thread using our shared memory section - attaching
			_data = shmat(shmid, NULL, 0);
			if (_data == (char *)(-1)) {
				perror("Data attachment failed");
				exit(1);
			}
		 
			//We write our answer:
			if(_myId == _sharedPID) {
				char answer[] = "OKAY";
				strncpy(_data, answer, SHM_SIZE);
			}
			else {
				char answer[] = "FALSE";
				strncpy(_data, answer, SHM_SIZE);
			}				
			
			//We gotta detach the memory.
			if (shmdt(_data) == -1) {
				perror("Failed detaching");
				exit(1);
			}
			printf("We wrote our data back for our parent. \n");
		 }
	 }
	 else {
		 //Parent thread.
		 printf("Our parent thread has the PID: %d \n", getpid());
		 //We now write the pid in the shared memory.
		 //We attach the memory section first
		 
		 data = shmat(shmid, NULL, 0);
		 if (data == (char *)(-1)) {
			perror("Data attachment failed");
			exit(1);
		 }
		 
		 //We write the actual process id in it
		 sprintf(data, "%d", _pid);
		 
		 //We detach it again.
		 if (shmdt(data) == -1) {
			perror("Failed detaching");
			exit(1);
		}
		printf("We wrote the PID and are sleeping now for a second to await our child's response \n");
				
		int awaitedInfo = -1;
		//We await information.
		
		char _response0[] = "OKAY";
		char _response1[] = "FALSE";
		
		while (awaitedInfo < 0) {
			sleep(1);
			
			data = shmat(shmid, NULL, 0);
			if (data == (char *)(-1)) {
				perror("Data attachment failed");
				exit(1);
			}
			
			//We write the actual process id in it
			if(strcmp(data, _response0) || strcmp(data, _response1)) {
				printf("We received the answer: %s \n", data);
				awaitedInfo = 1;
			}
			else {
				printf("We are still awaiting our child's response... Current data: %s \n", data);
			}
			
			//We detach it again.
			if (shmdt(data) == -1) {
				perror("Failed detaching");
				exit(1);
			}
		}
		
	 }
	 
	 return 0;
}
