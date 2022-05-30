#include <stdio.h> 
#include <string.h> 
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <unistd.h>

FILE * input_file;
ssize_t words_number;
ssize_t words;
int text;
int output;
size_t lenth = 150;
char * word;

void operation_starting_signal();
void operation_ending_signal();        


int main(int argc, char *argv[]) {
	output = atoi(argv[1]);																		
	text = atoi(argv[2]);                                           // convertig strings to intigers
    
    input_file = fopen("p1.txt", "r");                           // opening p1.txt file			

	kill(getppid(), SIGUSR1); 										// send a signal SIGUSR1 to "zadanie"
	
    signal(SIGUSR1, operation_starting_signal);						// SIGUSR1 signal to starting function 
		
	signal(SIGUSR2, operation_ending_signal);						// send a signal SIGUSR2 to "zadanie" informing about enging of operation
	
    while(1){	
		sleep(10);	
	}
    
	return 0;
}

void operation_starting_signal() {
	char * word = calloc(lenth, sizeof(char));						// allocating some memory for our operation
   
    words_number = getline(&word, &lenth, input_file); 				// getline function before coppying it ti otput
		
	words = write(output, word, strlen(word));	                // coppying by write funk
    	
    free(word);														// free our allocated memory
} 

void operation_ending_signal() {         
   	fclose(input_file);												// closing stream file to end our opeation 
} 
