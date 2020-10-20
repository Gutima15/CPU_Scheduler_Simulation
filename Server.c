#include <stdio.h> 
#include <netdb.h> 
#include <unistd.h>
#include <netinet/in.h> 
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 

#define PORT 8087
#define MAX __SIZEOF_INT__*3
#define SOCKETEROR (-1)
#define SERVER_BACKLOG 12 //Cantidad máxima de conexiones

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

struct process{
    int PID;
    int burst;
    int prioridad;
};

void handle_connection(int client_Socket);
int check(int exp, const char* mjs);


// Driver function
int main() 
{ 
	int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;
    //Create socket verification
    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)), "failed to create socket") 	
	printf("Socket successfully created..\n"); 
	//bzero(&servaddr, sizeof(servaddr)); // limpiamos la estructura del serverAdress, ojo que recibe la drección de memoria y la cantidad de lo que limpia

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	check((bind(server_socket, (SA*)&server_addr, sizeof(server_addr)), "bind feiled..")
    printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
    check((listen(server_socket, SERVER_BACKLOG)),"Listen failed")
	printf("Server listening..\n"); 
	
    while(true){
        printf("waiting for connections");
        //wait for, and eventually accept an incoming connection
        addr_size = sizeof(SA_IN);
        check(client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size) ,"Accept failed");
        printf("Connected");;
        // hacer nuestra lógica con conexiones
        handle_connection(client_socket);
    }
    return 0;
} 
int check(int exp, cont char *mjs){
    if(exp == SOCKETEROR){
        perror(mjs);
        exit(1);
    }
    return exp;
}
void handle_connection(int client_Socket){
    char buffer[MAX];
    size_t bytes_read;
    int mjsSize= 0;
    char actualpath[PATH_MAX+1]
    //leemos el mjs del cliente
    while (bytes_read = read(client_Socket, buffer+mjsSize, sizeof(buffer)-mjsSize-1) >0 )
    {
        mjsSize += bytes_read;
        if(mjsSize > BUFSIZE-1 || buffer[mjsSize-1]== "\n") break;        
    }
    check(bytes_read, "Recv error");
    buffer[mjsSize-1] = 0; // terminate the mjs and remove the enter
    printf ("REQUEST: %s\n",buffer);
    fflush(stdout);

    if(realpath(buffer,actualpath) == NULL){
        printf("Error(bad path): %s\n", buffer);
        close(client_Socket);
        return;
    }

    //lee el path del cliente
    FILE *fp = fopen(actualpath, "r");
    if(fp == NULL){
        printf("Error(open): %s\n", buffer);
        close(client_Socket);
        return;
    }

    //leemos y se lo enviamos al cliente
    while ( (bytes_read = fread(buffer,1,BUFSIZE,fp)) > 0){
        printf("Sending %zu bytes\n", bytes_read);
        write(client_Socket,buffer,bytes_read);
    }
    close(client_Socket);
    fclose(fp);
    printf("closing connection\n");

    
}
