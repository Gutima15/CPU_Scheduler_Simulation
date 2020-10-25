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
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#define PORT 8087
#define MAX 10
#define SOCKETEROR (-1)
#define SERVER_BACKLOG 12 //Cantidad máxima de conexiones
#define NTHREADS 64

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;
struct PCB{
    int PID;
    int burst;
    int prioridad;
    int tLlegada;
    int tSalida;
};
typedef struct node {
    struct PCB *val;
    struct node * next;
} node_ready;

int TiempoGlobal =0;
int CPUOcioso =0;
int cantProcesos=0;
int PID = 1;
node_ready *ready_queue =NULL;
node_ready *finish_queue =NULL;
bool flag= true;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// queue finished;


void print_list(node_ready * head) {        
    if(head != NULL){
        node_ready * current = head;    
        printf("HEAD MEMORY BEFORE %p\n",head);
        while (current != NULL) {
            puts("dentro del print list while");
            printf("PID: %d\t Burst: %d\t Priority: %d\n", current->val->PID, current->val->burst, current->val->prioridad);
            current = current->next;
        }
        printf("HEAD MEMORY AFTER %p\n",head);
    }                    
}

//Ingresa valores al final de la cola
node_ready* push(node_ready * head, struct PCB val) {
    if(head == NULL){
        head = (node_ready *) malloc(sizeof(node_ready));
        bzero(head, sizeof(head));
        head->val = (struct PCB*) malloc(sizeof(struct PCB));
        bzero(head->val, sizeof(head->val));
        head->val->PID = val.PID;
        head->val->burst = val.burst;
        head->val->prioridad = val.prioridad;
        head->val->tLlegada = val.tLlegada;
        head->val->tSalida = val.tSalida;
        head->next =NULL;        
    } else{
        node_ready * current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        /* now we can add a new variable */
        current->next = (node_ready *) malloc(sizeof(node_ready));
         current->next->val = (struct PCB*) malloc(sizeof(struct PCB));
        current->next->val->PID = val.PID;
        current->next->val->burst = val.burst;
        current->next->val->prioridad = val.prioridad;
        current->next->val->tLlegada = val.tLlegada;
        current->next->val->tSalida = val.tSalida;
        current->next->next = NULL;
    }
    return head;
}

//Remueve valores al inicio de la cola
struct PCB *pop(node_ready ** head) {
    struct PCB *retval = NULL;
    node_ready * next_node = NULL;

    if (*head == NULL) {
        return NULL;
    }

    next_node = (*head)->next;
    retval = (*head)->val;
    free(*head);
    *head = next_node;

    return retval;
}


void* job_scheduler(void* client_Socket);
void* consult_queue(void* queue);
void changemode(int);
int  kbhit(void);

int check(int exp, const char* mjs);
int kbhit (void)
{
  struct timeval tv;
  fd_set rdfs;

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  FD_ZERO(&rdfs);
  FD_SET (STDIN_FILENO, &rdfs);

  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &rdfs);
}
void changemode(int dir){
  static struct termios oldt, newt;

  if ( dir == 1 )
  {
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
  }
  else
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}
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

    //selecciona el algoritmo
    //puts("press any key to consult the ready queue.")
    //sleep(3)
    while(true){
        printf("waiting for connections\n");
        //wait for, and eventually accept an incoming connection
        addr_size = sizeof(SA_IN);
        check(client_socket = 
                accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size),
                "Accept failed");
        printf("Connected\n");    
        // hacer nuestra lógica con conexiones        
        pthread_t t;
        int*pclient = malloc(sizeof(int));
        *pclient = client_socket;        
        pthread_create(&t,NULL,job_scheduler,pclient);
        
        
        //Thread de la ready queue.
        pthread_t tqueue;
        pthread_create(&tqueue,NULL,consult_queue,ready_queue);        
        //pthread_join(&t,NULL);
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
    
    while (bytes_read = read(client_Socket, buffer, sizeof(buffer)) >0 )
    {
        check(bytes_read, "Recv error");
        buffer[MAX] = '\0'; // terminate the mjs and remove the enter

        fflush(stdout);
        //Se crea la estructura          15                3
        char* p = buffer;
        char* midPoint;
	    long int burst = strtol(p,&midPoint,10);
	    p = midPoint;
	    long int priority = strtol(p,&midPoint,10);        
        struct PCB p_c_b = {PID, (int)(burst) , (int)(priority) ,TiempoGlobal,0};                
        //Agregarla a la cola
        pthread_mutex_lock(&lock);
        ready_queue = push(ready_queue,p_c_b);        
        pthread_mutex_unlock(&lock);
        bzero(buffer, sizeof(buffer)); // limpiamos el buffer
		sprintf(buffer,"%d",PID);
        write(client_Socket,buffer,sizeof(buffer));   
        close(client_Socket); 
        printf("closing connection\n");
        PID++;
    }     
    return NULL;
    
}

void* consult_queue(void* queue){    
      
    if (kbhit() && flag){
    flag = !flag;
    node_ready rq = *((node_ready*)queue);
    free(queue); //We don't need it anymore..
    print_list(&rq);
    }
    
    
}