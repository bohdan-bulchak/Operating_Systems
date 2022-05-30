#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <sys/wait.h>

void operation_ending_signal(); 

int main (int argc, char* argv[]){
    	char *buffer;
   	int temp_text;

	int memory_pID = atoi(argv[1]); 	// memory for reading 
	int semaphore_pID = atoi(argv[2]); 	// semafor memory
	int port = atoi(argv[3]);  			// port number

	struct sembuf sembuf_struct[1]; 	// semafor structure 

	buffer = shmat(memory_pID, NULL, 0); 		// accessing memory by smat function (atteching it to empty one, so creating a new one) and checking if no failures comes 
		
	
	kill(getppid(), SIGUSR1);   				// sending a signal SIGUSR1 to "zadanie" 

	signal(SIGUSR2, operation_ending_signal);    // sending a signal informing about ending of operation
																			
	int new_socket;                              // starting with TSP settings (as a client side)(socket->connect->write)

	struct sockaddr_in adress; 

	new_socket = socket(AF_INET, SOCK_STREAM, 0);		// open a socket
													    // setting socket structure
	adress.sin_family = AF_INET;                        // ipv4 protocol setting
	adress.sin_port = htons(port);						// attaching socket to the port
	adress.sin_addr.s_addr = inet_addr("127.0.0.1");	// adress setting
	bzero(&(adress.sin_zero), 8);

	connect(new_socket, (struct sockaddr*) &adress, sizeof (adress)); 	// calling connects the socket to the adress (setted above)

	temp_text = open("temp_buffer_d.txt",O_CREAT|O_TRUNC|O_WRONLY,0777);	// temp text file with words for bussering it

	while(1){
		
		sembuf_struct[0].sem_num = 1;       // setting semafor on value "0" (red -> have to wait) 
		sembuf_struct[0].sem_op = -1;
		sembuf_struct[0].sem_flg = SEM_UNDO;
		semop(semaphore_pID, sembuf_struct, 1);	    // operations on semafor with semid "pid_semafor", seting atributes above to it

		if (write(new_socket, buffer, strlen(buffer)+1) != (int)strlen(buffer)+1){					// write to socket and checking on its success
			exit(EXIT_FAILURE);
        } else {
            sleep(1);
            write(temp_text, buffer,strlen(buffer));      // writing to buffer 
            write(temp_text, "\n", 1);
        }
		
		
		sembuf_struct[0].sem_num = 0;       // setting semafor on value "1" (green -> continue)
		sembuf_struct[0].sem_op = 1;
		sembuf_struct[0].sem_flg = SEM_UNDO;
		semop(semaphore_pID, sembuf_struct, 1);     // operations on our semafor, setting atributes above (for value 0) to it 
	}
	
}

void operation_ending_signal() {
   	exit(EXIT_SUCCESS);
}

