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
#include <pthread.h>
#define PORT 8087
#define MAX 10
#define SOCKETEROR (-1)
#define SERVER_BACKLOG 12 //Cantidad máxima de conexiones
#define NTHREADS 64

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;
int TiempoGlobal =0;
int CPUOcioso =0;
int cantProcesos=0;
int PID = 1;
// queue Ready;
// queue finished;
struct PCB{
    int PID;
    int burst;
    int prioridad;
    int tLlegada;
    int tSalida;
};

void* job_scheduler(void* client_Socket);
int check(int exp, const char* mjs);


// Driver function
int main() 
{ 
	int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;
    //Create socket verification
    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)), "failed to create socket");
	printf("Socket successfully created..\n"); 
	//bzero(&server_addr, sizeof(servaddr)); // limpiamos la estructura del serverAdress, ojo que recibe la drección de memoria y la cantidad de lo que limpia

	// assign IP, PORT 
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = INADDR_ANY; 
	server_addr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	check((bind(server_socket, (SA*)&server_addr, sizeof(server_addr))), "bind feiled..");
    printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
    check((listen(server_socket, SERVER_BACKLOG)),"Listen failed");
	printf("Server listening..\n"); 		
    while(true){
        printf("waiting for connections\n");
        //wait for, and eventually accept an incoming connection
        addr_size = sizeof(SA_IN);
        check(client_socket = 
                accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size) ,
                "Accept failed");
        printf("Connected\n");    
        // hacer nuestra lógica con conexiones        
        pthread_t t;
        int*pclient = malloc(sizeof(int));
        *pclient = client_socket;        
        pthread_create(&t,NULL,job_scheduler,pclient);
        //Thread del CPU Scheduler
    }
    return 0;
} 
int check(int exp, const char *mjs){
    if(exp == SOCKETEROR){
        perror(mjs);
        exit(1);
    }
    return exp;
}
void* job_scheduler (void* p_client_Socket){
    
    int client_Socket = *((int*)p_client_Socket);
    free(p_client_Socket); //We don't need it anymore..
    char buffer[MAX];
    size_t bytes_read; 
    //leemos el mjs del cliente
    
    while (bytes_read = read(client_Socket, buffer, sizeof(buffer)) >0 )
    {
        check(bytes_read, "Recv error");
        buffer[MAX] = '\0'; // terminate the mjs and remove the enter
        printf ("\nREQUEST: %s\n",buffer);
        fflush(stdout);
        //Se crea la estructura          15                3
        char* p = buffer;
        char* midPoint;
	    long int burst = strtol(p,&midPoint,10);
	    p = midPoint;
	    long int priority = strtol(p,&midPoint,10);        
        struct PCB p_c_b = {PID, (int)(burst) , (int)(priority) ,TiempoGlobal,0};                
        //Agregarla a la cola
        bzero(buffer, sizeof(buffer)); // limpiamos el buffer
		sprintf(buffer,"%d",PID);
        printf("Esto se le envía al cliente %s\n",buffer);
        write(client_Socket,buffer,sizeof(buffer));   
        close(client_Socket); 
        printf("closing connection\n");
        PID++;
    }
    // read(client_Socket, buffer, sizeof(buffer));
    // buffer[4] = '\0'; // terminate the mjs and remove the enter
    // printf ("\nREQUEST: %s\n",buffer);
    // fflush(stdout);
    // printf("Esto se envía al cliente: %s\n",buffer);    
    // write(client_Socket,buffer,sizeof(buffer));    
    //Aquí viene el buffer con los datos brust prioridad, se le deba dar el pci al proceso...        
    return NULL;
    
}
