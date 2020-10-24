#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <pthread.h>
#include <time.h> 
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#define MAX 6
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

int kbhit(void){
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

void ClienteManual(char* path) 
{ 		
	FILE *fp;
	fp = fopen(path, "r");
    if(fp == NULL){
        printf("Error(open): %s\n", path);        
        return;
    }			
	char line[MAX];
	pthread_t threads[NTHREADS];
  	void * retvals[NTHREADS];	
	char* lines[NTHREADS];
	int pos =0;
    while (fgets(line, MAX, fp) != NULL){
		if (line[0] != 'b' && line[0] != 'B'){
			lines[pos] = (char *) malloc(6);
			strncpy(lines[pos],line,6);
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
	char valueFromUser[8];
	char timebp[3];
	char randomLine[MAX+1];
	int burstAnswer;
	int timeBetweenProcess;
	burstAnswer= getLine ("Enter the max value of the random burst: ", valueFromUser, sizeof(valueFromUser)); 
	// validación para ver si es mayor a 2 dígitos
	timeBetweenProcess = getLine ("Enter the value between process creation: ", timebp, sizeof(timebp)); 
	time_t t;
	srand((unsigned) time(&t));
	
	pthread_t threads[NTHREADS];
  	void * retvals[NTHREADS];	
	char* lines[NTHREADS];
	int pos =0;	
    while (!kbhit()){		//Arreglar ciclo
		puts("antes del burst");
		printf("Max burst: %s, tamaño: %d\n",valueFromUser, sizeof(valueFromUser));
		int randomMax = atoi(valueFromUser);
		int num = (rand() %(randomMax)+1); 	
		printf("burst: %d\n",num);
		char numstr[3];
		sprintf(numstr,"%d",num);	
		strcpy(randomLine,"");	
		
		strcat(randomLine,numstr);  
		strcat(randomLine," ");
		
		bzero(numstr,sizeof(numstr));
		num = (rand()%5)+1;
		sprintf(numstr,"%d",num);
		strcat(randomLine,numstr);
		bzero(numstr,sizeof(numstr));
		
		lines[pos] = (char *) malloc(6);
		strncpy(lines[pos],randomLine,6);
		
		pthread_create(&threads[pos],NULL,ThreadLogic,lines[pos]);				
		pos++;			
		sleep(atoi(timebp)); 

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
} 

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
        printf ("You have select #%s. The manual client.\n", buff);    		
    	char path[4096];
		rc = getLine ("Enter the path of the process file: ", path, sizeof(path));
		ClienteManual(path);
    }
	if (buff[0]=='2'){
		printf ("Your have select #%s. The automatic client.\n");
		ClienteAutomatico();
	}
	if (buff[0]!='1' && buff[0]!='2'){
		puts("Incorrect data. Try again.\n");
	}
    
	// close the socket 
	
	return 0;
} 
//ṕara ver threads ps -a -T 