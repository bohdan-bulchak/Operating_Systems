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
#include <sys/wait.h>



bool p1_Done = false;
bool p2_Done = false;
bool t_Done = false;
bool d_Done = false;
bool s_Done = false;
bool end = false;
bool d_Operating = true;
bool serv1_Done = false;
bool serv2_Done = false;

void p1_done();

void p2_done();

void t_done();

void s_done();

void serv1_done();

void d_done();

void serv2_done();

void d_operating();

void end_operating();

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
}semafor_struct;


int main( int argc, char *argv[]) {
	 
	pid_t p1 = 0,p2 = 0,pr = 0,t = 0, d = 0, s = 0, serv1 = 0, serv2 = 0; 		//pomocne premenne pre pidy child procesov
	char R1_read[12],R1_write[12],R2_read[12],R2_write[12],pp1[12],pp2[12],pamat_1[12],pamat_2[12],semafor_1[12],semafor_2[12];			//pomocne premenne pre stringy argumentov

	
	int R1_pipe[2];		    //input pipe R1
	int R2_pipe[2]; 		//output pipe R2	
	pipe(R1_pipe);  		//make a R1 pipe(p1 and p2 proccesses)
	pipe(R2_pipe);		    //make a R2 pipe(pr proccess)

	//converting pipe context int to string
	sprintf(R1_read, "%d", R1_pipe[0]);		 
	sprintf(R1_write, "%d", R1_pipe[1]);
	sprintf(R2_read, "%d", R2_pipe[0]);
	sprintf(R2_write, "%d", R2_pipe[1]);
	
	//proc_p1 opening as a child proccess
	signal(SIGUSR1, p1_done);           // p1 signal reciever 
	switch (p1 = fork()) {
		case  0:
			printf("Spustam proces P1\n");
			execl("proc_p1", "proc_p1", R1_write, R1_read, NULL);
			break;
		case -1:printf("ERROR: p1 nebol spusteny!\n");
			break;
		default: printf("Switch P1 done\n");
	}	
	while(!p1_Done){printf("Cakam na spustenie P1\n"); sleep(2);};
    

	//proc_p2 opening as a child proccess
	signal(SIGUSR1, p2_done);           // p1 signal reciever
    switch (p2 = fork()) {
		case  0:
			printf("Spustam proces p2\n");
			execl("proc_p2", "proc_p2", R1_write, R1_read, NULL);
			break;
		case -1:printf("ERROR: p2 nebol spusteny!\n");
			break;
		default: printf("Switch P2 done\n");
	}
	while(!p2_Done){printf("Cakam na spustenie P2\n"); sleep(2);};
     

	//proc_p2 opening as a child proccess

    switch (pr = fork()) {
		case  0:
			sprintf(pp1, "%d", p1);
			sprintf(pp2, "%d", p2);
			printf("Spustam PR\n");
			execl("proc_pr", "proc_pr", pp1, pp2, R1_read, R2_write, NULL);
			break;
		case -1:printf("ERROR: PR nebol spusteny!\n");
			break;
		default: printf("Switch PR done\n");
	}	

	int state;
	waitpid(pr, &state, WUNTRACED);   // pr ending proccess waiting
	
	// ending of p1 and p2 proccesses
    	kill(p1,SIGUSR2);
	kill(p2,SIGUSR2);
	
	//creating 1 a 2 zdielane pamati :
	int pamat_t;
	int pamat_d;

    if ((pamat_t = shmget(2005, 150*sizeof(char), 0666 | IPC_CREAT)) == -1){
		printf("ERROR: Nepodarilo sa vytvorit pamat_t!\n");
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}
	else printf("pamat_t bola vytvorena. ID: %d\n",pamat_t);
	

	if ((pamat_d = shmget(2007, 150*sizeof(char), 0666 | IPC_CREAT)) == -1){
		printf("ERROR: Nepodarilo sa vytvorit pamat_d!\n");
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}
	else printf("pamat_d bola vytvorena. ID: %d\n",pamat_d);

	// creating semaphores

	int semafor_t;
	int semafor_d;

    if ((semafor_t = semget(4001, 2, IPC_CREAT | 0666)) == -1){
		printf("ERROR: Nepodarilo sa vytvorit semafor_t!\n");
		fprintf(stderr, "semget failed\n");
		exit(EXIT_FAILURE);
	}

	semafor_struct.val = 1;                            //semaphores structures 
	semctl (semafor_t, 0, SETVAL, semafor_struct);
	semafor_struct.val = 0;
	semctl (semafor_t, 1, SETVAL, semafor_struct);
 
    if ((semafor_d = semget(4010, 2, IPC_CREAT | 0666)) == -1){
		printf("ERROR: Nepodarilo sa vytvorit semafor_d!\n");
		fprintf(stderr, "semget failed\n");
		exit(EXIT_FAILURE);
	}

	semafor_struct.val = 1;
	semctl(semafor_d, 0, SETVAL, semafor_struct);
	semafor_struct.val = 0;
	semctl(semafor_d, 1, SETVAL, semafor_struct);

	// serv2 signals reciervers 
    
	signal(SIGUSR1, serv2_done);
	signal(SIGUSR2, end_operating); 

	//serv2 opening as a child proccess

    switch (serv2 = fork()) {
		case  0:
			printf("Spustam proces Serv2...\n");
			execl("proc_serv2", "proc_serv2", argv[2],NULL);
			break;
		case -1:printf("ERROR: Proces Serv2 nebol spusteny!\n");
			break;
		default: printf("Switch SERV2 done\n");
	}
	while(!serv2_Done){printf("Cakam na spustenie Serv2\n"); sleep(2);};


	//proc_p1 opening as a child proccess
    	signal(SIGUSR1, serv1_done); 

    switch (serv1 = fork()) {
		case  0:
			printf("Spustam proces Serv1...\n");
			execl("proc_serv1", "proc_serv1", argv[1], argv[2],NULL);
			break;
		case -1:printf("ERROR: Proces Serv1 nebol spusteny!\n");
			break;
		default: printf("Switch SERV1 done\n");
	}
	while(!serv1_Done){printf("Cakam na spustenie Serv1\n"); sleep(2);};

	
	//proc_t opening as a child proccess
    	signal(SIGUSR1, t_done); 

    switch (t = fork()) {
		case  0:
			printf("Spustam proces T...\n");
			sprintf(semafor_1, "%d", semafor_t);
			sprintf(pamat_1, "%d", pamat_t);
			execl("proc_t", "proc_t", R2_read, pamat_1, semafor_1, NULL);
			break;
		case -1:printf("ERROR: T nebol spusteny!\n");
			break;
		default: printf("Switch PT done\n");
	}
    while(!t_Done){printf("Cakam na spustenie PT\n"); sleep(2);};
			
	//proc_d opening as a child proccess
    	signal(SIGUSR1, d_done); 
    
    switch (d = fork()) {
		case  0:
			printf("Spustam proces D...\n");
			sprintf(pamat_2, "%d", pamat_d);
			sprintf(semafor_2, "%d", semafor_d);
			execl("proc_d", "proc_d", pamat_2, semafor_2, argv[1], NULL);
			break;
		case -1:printf("ERROR: T nebol spusteny!\n");
			break;
		default: printf("Switch PD done\n");
	}
	while(!d_Done){printf("Zadanie: Cakam na spustenie PD\n"); sleep(2);};


	//proc_s opening as a child proccess		
    	signal(SIGUSR1, s_done);

    switch (s = fork()) {
		case  0:
			printf("Spustam proces S...\n");
			sprintf(semafor_1, "%d", semafor_t);
			sprintf(pamat_1, "%d", pamat_t);
			sprintf(pamat_2, "%d", pamat_d);
			sprintf(semafor_2, "%d", semafor_d);
			execl("proc_s", "proc_s", pamat_1, semafor_1, pamat_2, semafor_2, NULL);
			break;
		case -1:printf("ERROR: S nebol spusteny!\n");
			break;
		default: printf("Switch PS done\n");
	}
 
	while(!s_Done){printf("Cakam na spustenie PS\n"); sleep(2);};

    //serv2 closing
    while(!end){printf("Zadanie: Cakam na ukoncenie Serv2\n"); sleep(2);};
			

	

	//close t, d and s proccesses
	kill(t, SIGUSR2);
	kill(d, SIGUSR2);
	kill(s, SIGTERM);

	// ports closing
	close(atoi(argv[2]));
	close(atoi(argv[1]));	

    printf("KONIEC zadania\n");
	return 0; 
} 

void p1_done(){
    if(!p1_Done) printf("P1 spusteny!\n"); 
	p1_Done = true;
}

void p2_done(){
    if(!p2_Done && p1_Done) printf("P2 spusteny!\n"); 
	p2_Done = true;
}

void t_done(){
    if(!t_Done && p2_Done && p1_Done) printf("PT spusteny!\n"); 	
	t_Done = true;
}

void s_done(){
    if(!s_Done && d_Done && t_Done && p2_Done && p1_Done) printf("PS spusteny!\n"); 
	s_Done = true;
}

void serv1_done(){
    printf("Serv1 spusteny!\n"); 
	serv1_Done = true;
}

void d_done(){
    if(!d_Done && serv1_Done && t_Done && p2_Done && p1_Done) printf("PD spusteny!\n"); 
	d_Done = true;
}

void serv2_done(){
    if(!serv2_Done && serv1_Done && t_Done && p2_Done && p1_Done) printf("Serv1 spusteny!\n"); 
	serv2_Done = true;
}

void d_operating(){
    printf("PD is proccessing!\n");
	d_Operating = false;
}

void end_operating(){
    printf("Ukoncenie Zadania\n");
	end = true;
}
