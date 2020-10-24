#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <pthread.h>
#include <time.h> 
#define MAX __SIZEOF_INT__*3
#define PORT 8087 
#define SA struct sockaddr 
#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2
#define NTHREADS 64

pthread_mutex_t lock= PTHREAD_MUTEX_INITIALIZER;
struct socketInfo{ 
    int sfd; 
	char* b;
};
void* ThreadLogic(char* data );

static int getLine (char *prmpt, char *buff, size_t sz) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }
    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
    return OK;
}

void ClienteManual(char* path) 
{ 	
	
	FILE *fp;
	fp = fopen(path, "r");
    if(fp == NULL){
        printf("Error(open): %s\n", path);        
        return;
    }			
	char line[4];
	pthread_t threads[NTHREADS];
  	void * retvals[NTHREADS];	
	char* lines[NTHREADS];
	int pos =0;
    while (fgets(line, MAX, fp) != NULL){
		if (line[0] != 'b' && line[0] != 'B'){						
			lines[pos] = (char *) malloc(4);
			strncpy(lines[pos],line,4);
			printf("He leido: %s\n",lines[pos]);			
			pthread_create(&threads[pos],NULL,ThreadLogic,lines[pos]);				
			pos++;
		}		
		time_t t;
		srand((unsigned) time(&t));
		int num = (rand() %(8 - 3 + 1)) + 3; 	
		sleep(num); 
	}
	fclose(fp);

	for (int i = 0; i < pos; i++){
      if (pthread_join(threads[i], &retvals[i]) != 0)
        {
          fprintf(stderr, "error: Cannot join thread # %d\n", i);
        }
    }    
    return NULL;
} 

void ClienteAutomatico(){
	char randomLine[4];
	int burstAnswer;
	burstAnswer= getLine ("Enter the max value of the random burst: ", randomLine, sizeof(randomLine));

	//strtol
	pthread_t threads[NTHREADS];
  	void * retvals[NTHREADS];	
	char* lines[NTHREADS];
	int pos =0;
    while (1){
		lines[pos] = (char *) malloc(4);
		strncpy(lines[pos],randomLine,4);
		printf("He leido: %s\n",lines[pos]);			
		pthread_create(&threads[pos],NULL,ThreadLogic,lines[pos]);				
		pos++;
				
		time_t t;
		srand((unsigned) time(&t));
		int num = (rand() %(8 - 3 + 1)) + 3; 	
		sleep(num); 
	}

	for (int i = 0; i < pos; i++){
      if (pthread_join(threads[i], &retvals[i]) != 0)
        {
          fprintf(stderr, "error: Cannot join thread # %d\n", i);
        }
    }    
    return NULL;
}

void* ThreadLogic(char* data){	
	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 
	srand(time(0));
	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	servaddr.sin_port = htons(PORT); 

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { // C
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else
		printf("connected to the server..\n"); 
	//--------------------
	int rest =0;
	printf("He leido del thread: %s\n",data);
	sleep(2); //los 2 segundos de sleep antes de enviar los datos.
	write(sockfd, data, sizeof(data)); 
	//recibimiento
	
	read(sockfd, data, sizeof(data)); 
	printf("From Server : %s\n", data);
	close(sockfd);
} // 5 3 --> PID

int main() 
{ 

	//Menú de selección de cliente
	int rc;
    char buff[2];

    rc = getLine ("Enter 1 for manual client or 2 for automatic client: ", buff, sizeof(buff));
    if (rc == NO_INPUT) {
        // Extra NL since my system doesn't output that on EOF.
        printf ("\nNo input, try again.\n");
        
    }

    if (rc == TOO_LONG) {
        printf ("Input too long, try again [%s]\n", buff);
        
    }
    if (buff[0]== '1'){
        printf ("You select #%s the manual client.\n", buff);    		
    	char path[4096];
		rc = getLine ("Enter the path of the process file: ", path, sizeof(path));
		ClienteManual(path);
    }
    
	// close the socket 
	
	return 0;
} 
//ṕara ver threads ps -a -T 