#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  

int main(int argc, char* argv[]){
	char buffer[300];

	size_t lenth = 300;
	int port= atoi(argv[1]);        // converting port param string to intiger
	int output_file;
									// starting with UDP settings (socket->bind->recvfrom->write)
	struct sockaddr_in adress; 
	int new_socket; 
	new_socket = socket(AF_INET, SOCK_DGRAM, 0); 		// open a socket, an checking on it success 
														// setting socket structure
	adress.sin_family = AF_INET;        // ipv4 protocol setting
	adress.sin_port = htons(port);      // attaching socket to the port
	adress.sin_addr.s_addr = INADDR_ANY;        // adress setting

	bind(new_socket, (struct sockaddr *) &adress, sizeof(adress));       // assigning the address to created socket by bind func
																	
	kill(getppid(),SIGUSR1);        // sending a SIGUSR1 signal to "zadanie" to successful continuing

    	output_file = open("serv2.txt", O_CREAT|O_TRUNC|O_WRONLY, 0777);      // open a file serv2.txt for coppying
	

	for (int i = 0; i < 10; i++) {																	
		recv(new_socket, buffer, lenth, 0);	        // receiving a massage from our socket "sockd" to buff 
		write(output_file, buffer, strlen(buffer));	        // writing strings to our fd (opened serv2.txt) 
		write(output_file, "\n", 1);
	}

	kill(getppid(), SIGUSR2);        // sending a signal informing about ending of operation

   	return 0;
}
