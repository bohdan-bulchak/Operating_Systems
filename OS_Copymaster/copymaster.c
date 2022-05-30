#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <fcntl.h> 
#include <unistd.h>



#include "options.h"


int outputFileChecking(struct CopymasterOptions* cpm_options) {
	int outputFile;
	struct stat inf;
	stat(cpm_options -> infile, &inf);
	
	if (cpm_options -> create) {   // -c condition
	
		outputFile = open(cpm_options -> outfile, O_WRONLY | O_CREAT | O_EXCL, cpm_options -> create_mode);
	
		if (outputFile == -1){ 
			FatalError('c',"SUBOR EXISTUJE", 23);
		}	
	}
	
	else if (cpm_options -> overwrite){  //-o condition
	
		outputFile = open(cpm_options -> outfile, O_WRONLY | O_TRUNC);
	
		if (outputFile == -1) {
            		FatalError('o',"SUBOR NEEXISTUJE", 24);
		}
	}

	else if (cpm_options -> append) { // -a condition

		outputFile = open(cpm_options -> outfile, O_WRONLY | O_APPEND);

		if (outputFile == -1) {
		
		 	FatalError('a', "SUBOR NEEXISTUJE", 22);
		
		}
	}
	else {   //default option
	
		outputFile = open(cpm_options -> outfile, O_WRONLY | O_TRUNC | O_CREAT, 0666);
	
		if (outputFile == -1) {
			FatalError('-', "SUBOR NEEXISTUJE", 21);
		
		}
	
	}
	
	return outputFile;
}

void FatalError(char c, const char* msg, int exit_status);
void PrintCopymasterOptions(struct CopymasterOptions* cpm_options);


int main(int argc, char* argv[])
{
    struct CopymasterOptions cpm_options = ParseCopymasterOptions(argc, argv);

    //-------------------------------------------------------------------
    // Kontrola hodnot prepinacov
    //-------------------------------------------------------------------

    // Vypis hodnot prepinacov odstrante z finalnej verzie
    
    PrintCopymasterOptions(&cpm_options);
    
    //-------------------------------------------------------------------
    // Osetrenie prepinacov pred kopirovanim
    //-------------------------------------------------------------------
    
    if (cpm_options.fast && cpm_options.slow || 
        cpm_options.create && cpm_options.overwrite ||
        cpm_options.create && cpm_options.append ||
        cpm_options.append && cpm_options.overwrite) {

        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        
        exit(EXIT_FAILURE);
    
    }

    
    


	

    // TODO Nezabudnut dalsie kontroly kombinacii prepinacov ...
    
    //-------------------------------------------------------------------
    // Kopirovanie suborov
    //-------------------------------------------------------------------
    
    // TODO Implementovat kopirovanie suborov
    
    // cpm_options.infile
    // cpm_options.outfile
    
    //-------------------------------------------------------------------
    // Vypis adresara
    //-------------------------------------------------------------------
    
    /////////////Kopirovanie bez prepinacov////////

    if (!cpm_options.slow && !cpm_options.fast && !cpm_options.lseek && 
        !cpm_options.directory && !cpm_options.delete_opt && 
        !cpm_options.inode && !cpm_options.link && !cpm_options.truncate &&
        !cpm_options.chmod && !cpm_options.umask) {
        int inputFile;
        int outputFile;
        char buffer;
        int new1;
        
        inputFile = open(cpm_options.infile,O_RDONLY);

        if (inputFile == -1){      // exist of inputFile checking
            FatalError('-', "SUBOR NEEXISTUJE", 21);
        }
    
        outputFile = outputFileChecking(&cpm_options);

        struct stat bufs;
        stat(cpm_options.outfile, &bufs);

        if ((bufs.st_mode & S_IRUSR) == 0) {
              FatalError('B', "INA CHYBA", 21);
        }

        long fileSize = lseek(inputFile, 0, SEEK_END);
        lseek(inputFile, 0, SEEK_SET);

        char buf [fileSize + 1];
        read(inputFile, buf, fileSize);
        write(outputFile, buf, fileSize);

        
        close(inputFile);
        close(outputFile);
    }


    ///////////// Slow Kopirovanie -s  ////////

    if (cpm_options.slow) {
        int inputFile;
        int outputFile;
        char buf;
        
        inputFile = open(cpm_options.infile,O_RDONLY);

        if (inputFile == -1){  //exist of inputFile checking
            FatalError('s', "INA CHYBA", 21);
        }
    
        outputFile = outputFileChecking(&cpm_options);

        while( read(inputFile, &buf, 1) > 0){  // 1 byte
            write(outputFile, &buf, 1);
        }
    
        close(inputFile);
        close(outputFile);
    }
   

   ///////////// Fast Kopirovanie -f  ////////

   if (cpm_options.fast) {
        int inputFile;
        int outputFile;
       
        inputFile = open(cpm_options.infile,O_RDONLY);

       if (inputFile == -1) {
           FatalError('f', "INA CHYBA", 21);
        }
        
        outputFile = outputFileChecking(&cpm_options);

        struct stat buf;
        
        fstat(inputFile, &buf);// write inputFile content to buf
    
        char buffer[buf.st_size];
    
        read(inputFile, &buffer, buf.st_size);
        write(outputFile, &buffer, buf.st_size);
    
        close(inputFile);
        close(outputFile);
    }

////////////// Lseek -l /////////////

    if (cpm_options.lseek) {
        int inputFile;
        int outputFile;
        int inPos;
        int outPos;

        outputFile = open(cpm_options.outfile, O_WRONLY);

        if( outputFile == -1){    //outputfile exist check
            FatalError('l', "INA CHYBA", 21);
        }

        inputFile = open(cpm_options.infile, O_RDONLY);
        
        if(inputFile == -1){     //inputfile exist check
            FatalError('l',"INA CHYBA", 21);
        }
        
        inPos = lseek(inputFile, cpm_options.lseek_options.pos1, SEEK_SET);

        if (inPos == -1) {       //infile position check
            FatalError('l',"CHYBA POZICIE infile", 33);
        }
        
        outPos = lseek(outputFile, cpm_options.lseek_options.pos2, cpm_options.lseek_options.x);

        if (outPos == -1) {//outfile position check
            FatalError('l',"CHYBA POZICIE outfile", 33);
        }

        int bytesCount = cpm_options.lseek_options.num;//copying request count of bytes
        
        char buffer[bytesCount];
        
        read(inputFile, &buffer, bytesCount);
        write(outputFile, &buffer, bytesCount);
        
        close(inputFile);
        close(outputFile);
    }

    /////// Directory -D ///////   //// nedokonceny!
    if (cpm_options.directory) {


        
        int chars;
        int inputFile;
        int outputFile;

        inputFile = fopen(cpm_options.infile, "r");
       
        if (inputFile == -1) {
           FatalError('D', "VSTUPNY SUBOR NIE JE ADRESAR", 28);
        }

        outputFile = fopen(cpm_options.outfile, "w");

        if (inputFile == -1) {
           FatalError('D', "VYSTUPNY SUBOR - CHYBA", 28);
        }

        while ((chars = fgetc(inputFile)) != EOF) {
            fputc(chars, outputFile);
           // chars++;
           // fprintf(outputFile, chars);
        }

        struct stat bufs;
        stat(cpm_options.infile, &bufs);

        //printf("%d ");

        close(inputFile);
        close(outputFile);


    }


    //////// Delete -d //////
 
    if (cpm_options.delete_opt) {
        int inputFile;
        int outputFile;
        int removeVal;

        inputFile = open(cpm_options.infile, O_RDONLY);

        if (inputFile == -1) {   
            FatalError('d', "SUBOR NEEXISTUJE", 21);
        }

        outputFile = open(cpm_options.outfile, O_WRONLY | O_CREAT);

        if (outputFile == -1) {
            FatalError('d', "INA CHYBA", 26);
        }

        unlink(cpm_options.infile);

        close(inputFile);
        close(outputFile);
    }


///////////// Inode -i ////////
    if(cpm_options.inode){
        struct stat atr;
        int inputFile;
        int outputFile;

        inputFile = open(cpm_options.infile, O_RDONLY);
      
        stat(cpm_options.infile, &atr); //Put inputFiles atributes in atr
        
        if (!S_ISREG(atr.st_mode)) {     // default file check
            FatalError('i', "ZLY TYP VSTUPNEHO SUBORU", 27);
        }
        
       
        if (atr.st_ino != cpm_options.inode_number) {  //condition of equality check 
            FatalError('i',"ZLY INODE", 27);
        }
        
        outputFile = outputFileChecking(&cpm_options);  //start of default(fast) copying

        struct stat buf;
        
        fstat(inputFile, &buf); // write inputFile content to buf
    
        char buffer[buf.st_size];
    
        read(inputFile, &buffer, buf.st_size);
        write(outputFile, &buffer, buf.st_size);
    }


    //////// Link -K //////////

    if (cpm_options.link) {
        int inputFile;
        int newLink;

        inputFile = open(cpm_options.infile, O_RDONLY);

        if( inputFile == -1) {
            FatalError('K', "VSTUPNY SUBOR NEEXISTUJE", 30);
        }
        
        if (link(cpm_options.infile, cpm_options.outfile) < 0 ) {
            FatalError('K', "VYSTUPNY SUBOR NEVYTVORENY", 30);
        }
    
        close(inputFile);
    }

    //////////// Truncate -t /////////////

    if (cpm_options.truncate) {
        int newTruncSize;
        
        if (cpm_options.truncate_size < 0) {    // size check
            FatalError('t', "ZAPORNA VELKOST", 31);
        }
        
        int inputFile = open(cpm_options.infile, O_RDONLY);
        int outputFile = open(cpm_options.outfile, O_WRONLY);
        
        char buffer;
        
        while (read(inputFile, &buffer, 1) > 0) {//make copy
            write(outputFile, &buffer, 1);
        }
        
        newTruncSize = truncate(cpm_options.infile, cpm_options.truncate_size);

        if (newTruncSize == -1) {
            FatalError('t', "INA CHYBA", 31);
        }
    }

    ///////////// Chmod -m ////////
    

    if (cpm_options.chmod) {
        int inputFile;
        int outputFile;
        int perm;
        struct stat atr;
        int chmodValue;

        inputFile = open(cpm_options.infile, O_RDONLY);

        if (inputFile == -1) {   
            FatalError('m', "SUBOR NEEXISTUJE", 21);
        }

        outputFile = outputFileChecking(&cpm_options);
       
        if (cpm_options.chmod_mode == 0) {
            FatalError('m', "ZLE PRAVA", 34);
        }

        chmodValue = chmod(cpm_options.outfile, cpm_options.chmod_mode);

        if (chmodValue == -1) {
            FatalError('m', "INA CHYBA", 34);
        }

                
        close(inputFile);
        close(outputFile);

    }


    //////////////// Umask -u /////////   // nedokonceny!!

    if (cpm_options.umask) {
        int inputFile;
        int outputFile;
        int chmodNum;
        int i;
        char actor = cpm_options.umask_options[i][0] ;
        char operation = cpm_options.umask_options[i][1];
        char  mode = cpm_options.umask_options[i][2];
        mode_t mask = umask(S_IRUSR | S_IRWXG | S_IRWXO);

        inputFile = open(cpm_options.infile, O_RDONLY);
        outputFile = outputFileChecking(&cpm_options);


        for (i = 0; i < 3; i++){
            if (actor == 'o' ) {
                if (operation == '-') {
                    if (mode == 'w') {
                        mask |= S_IWOTH;                        
                    }
                    else if (mode == 'r') {
                        mask |= S_IROTH;    
                    }
                    else {
                        mask |= S_IXOTH;

                    }
                } 
                else {
                    if (mode == 'w') {
                        mask &= (~S_IWOTH);    
                    }
                    else if (mode == 'r') {
                        mask &= (~S_IROTH);
                    }
                    else {
                        mask &= (~S_IXOTH);    
                    }
                }   
            }
            else if (actor == 'g') {
                if (operation == '-') {
                    if (mode == 'w') {
                        mask |= S_IWGRP;   
                    }
                    else if (mode == 'r') {
                        mask |= S_IRGRP;    
                    }
                    else {
                        mask |= S_IXGRP;    
                    }
                } 
                else {
                   if (mode == 'w') {
                       mask &= (~S_IWGRP);
                        
                    }
                    else if (mode == 'r') {
                        mask &= (~S_IRGRP);    
                    }
                    else {
                        mask &= (~S_IXGRP);    
                    }
                }   
            }
            else {
                if (operation == '-') {
                    if (mode == 'w') {
                        mask |= S_IWUSR;            
                    }
                    else if (mode == 'r') {
                        mask |= S_IRUSR;    
                    }
                    else {
                        mask |= S_IXUSR;    
                    }
                } 
                else {
                    if (mode == 'w') {
                        mask &= (~S_IWUSR);
                    }
                    else if (mode == 'r') {
                        mask &= (~S_IRUSR);    
                    }
                    else {
                        mask &= (~S_IXUSR);    
                    }
                }   
            }
        }

        umask(mask);


    close(inputFile);
    close(inputFile);
    }


        
    //-------------------------------------------------------------------
    // Osetrenie prepinacov po kopirovani
    //-------------------------------------------------------------------
    
    // TODO Implementovat osetrenie prepinacov po kopirovani
    
    return 0;
}


void FatalError(char c, const char* msg, int exit_status)
{
    fprintf(stderr, "%c:%d:%s:%s", c, errno, strerror(errno), msg); 
    //fprintf(stderr, ":%s", c, strerror(errno));
   // fprintf(stderr, ":%s", c, msg);
	
    exit(exit_status);
}

void PrintCopymasterOptions(struct CopymasterOptions* cpm_options)
{
    if (cpm_options == 0)
        return;
    
    printf("infile:        %s\n", cpm_options->infile);
    printf("outfile:       %s\n", cpm_options->outfile);
    
    printf("fast:          %d\n", cpm_options->fast);
    printf("slow:          %d\n", cpm_options->slow);
    printf("create:        %d\n", cpm_options->create);
    printf("create_mode:   %o\n", (unsigned int)cpm_options->create_mode);
    printf("overwrite:     %d\n", cpm_options->overwrite);
    printf("append:        %d\n", cpm_options->append);
    printf("lseek:         %d\n", cpm_options->lseek);
    
    printf("lseek_options.x:    %d\n", cpm_options->lseek_options.x);
    printf("lseek_options.pos1: %ld\n", cpm_options->lseek_options.pos1);
    printf("lseek_options.pos2: %ld\n", cpm_options->lseek_options.pos2);
    printf("lseek_options.num:  %lu\n", cpm_options->lseek_options.num);
    
    printf("directory:     %d\n", cpm_options->directory);
    printf("delete_opt:    %d\n", cpm_options->delete_opt);
    printf("chmod:         %d\n", cpm_options->chmod);
    printf("chmod_mode:    %o\n", (unsigned int)cpm_options->chmod_mode);
    printf("inode:         %d\n", cpm_options->inode);
    printf("inode_number:  %lu\n", cpm_options->inode_number);
    
    printf("umask:\t%d\n", cpm_options->umask);
    for(unsigned int i=0; i<kUMASK_OPTIONS_MAX_SZ; ++i) {
        if (cpm_options->umask_options[i][0] == 0) {
            // dosli sme na koniec zoznamu nastaveni umask
            break;
        }
        printf("umask_options[%u]: %s\n", i, cpm_options->umask_options[i]);
    }
    
    
    printf("link:          %d\n", cpm_options->link);
  

    printf("truncate:      %d\n", cpm_options->truncate); 
    printf("truncate_size: %ld\n", cpm_options->truncate_size);
    printf("sparse:        %d\n", cpm_options->sparse);
}
