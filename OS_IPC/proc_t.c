#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/shm.h> 

void operation_ending_signal(); 

int main (int argc, char* argv[]){
	char *buffer;
    	int temp_text;

    	int reading_pipe = atoi(argv[1]);							// memory for reading
	int memory_pID = atoi(argv[2]);	
	int semaphore_pID = atoi(argv[3]);							// semafor memory  // conveting strings parametres to intigers
	
	char output_text[300];

	struct sembuf sembuf_struct[1];								// semafor structure 

	buffer = shmat(memory_pID, NULL, 0);  					    // accessing memory by smat function (atteching it to empty one, so creating
								                                // a new one) and checking if no failures comes 

	kill(getppid(), SIGUSR1);									// sending a SIGUSR1 signal to "zadanie" to successful continuing 
	
	signal(SIGUSR2, operation_ending_signal);											// sending a signal informing about ending of operation

	temp_text = open("temp_buffer_t.txt", O_CREAT|O_TRUNC|O_WRONLY, 0777);           // temp text for coppying it to buffer zdiel. pamati

	char txt;
	int i = 0;
	
	while(1){													// working with S1 semafor
		
		sembuf_struct[0].sem_num = 0;							// setting semafor on value "0" (red -> have to wait) 
		sembuf_struct[0].sem_op = -1;
		sembuf_struct[0].sem_flg = SEM_UNDO;
		semop(semaphore_pID, sembuf_struct, 1);					// operations on semafor with semid "pid_semafor", seting atributes above to it

		while(txt != '\n'){										// first step of our coppying:
			read(reading_pipe, &output_text[i], 1);				// reading strings (words) from our pipe R2
			txt = output_text[i];
			i++;
		}
		txt = '\0';
        output_text[i-1] = '\0';
        strcpy(buffer, output_text);							// coppying readed words from temp_text file to our buffer zdiel. pam
	write(temp_text, buffer, strlen(buffer));
	write(temp_text, "\n", 1);
	i = 0;

	sembuf_struct[0].sem_num = 1;							// setting semafor on value "1" (green -> continue)
	sembuf_struct[0].sem_op = 1;	
	sembuf_struct[0].sem_flg = SEM_UNDO;
	semop(semaphore_pID, sembuf_struct, 1);					// operations on our semafor, setting atributes above (for value 0) to it 
	}
}

void operation_ending_signal() {
   	exit(EXIT_SUCCESS);
}
