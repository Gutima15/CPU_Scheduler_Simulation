#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <pthread.h>
#define MAX 4096
#define PORT 8087 
#define SA struct sockaddr 

void readFile(int sockfd) 
{ 
	char buff[MAX]; 
	int n; 
	bzero(buff, sizeof(buff)); 
	printf("Enter the path : "); 
	n = 0; 
	while ((buff[n++] = getchar()) != '\n') 
		; 

	fp = fopen(buff, "r");
    if(fp == NULL){
        printf("Error(open): %s\n", buffer);        
        return;
    }
	
    while (fgets(buff, MAX, fp) != NULL){
		//aquí va el thread
		pthread_t t;	
		pthread_create(&t,NULL,ThreadLogic, buff);		
	}

    fclose(fp);
    return;
} 

void* ThreadLogic(char* buffer ){
	char buff= *buffer;
	write(sockfd, buff, sizeof(buff)); 
   		 if ((strncmp(buff, "exit", 4)) == 0) { 
		printf("Client Exit...\n"); 
		return;
		}
		bzero(buff, sizeof(buff)); 
		read(sockfd, buff, sizeof(buff)); 
		printf("From Server : %s", buff); 
}

int main() 
{ 
	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 

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
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else
		printf("connected to the server..\n"); 

	// function for readFile 
	readFile(sockfd); 

	// close the socket 
	close(sockfd); 
} 
