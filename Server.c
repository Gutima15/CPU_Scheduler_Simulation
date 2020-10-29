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

static int getLine (char *prmpt, char *buff, size_t sz) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return 1;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? 2 : 0;
    }
    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
    return 0;
}
void print_list(node_ready * head) {
    if(head != NULL){
        node_ready * current = head;    
        while (current != NULL) {
            printf("PID: %d\t Burst: %d\t Priority: %d\n", current->val->PID, current->val->burst, current->val->prioridad);
            current = current->next;
        }
    }
    flag= !flag;                    
}

void print_process(struct PCB * pr){     
    if(pr != NULL){
        printf("PID: %d\t Burst: %d\t Priority: %d\n", pr->PID, pr->burst, pr->prioridad);
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

struct PCB *pop_spf(node_ready **head){ 
    struct PCB *retval = NULL;
    node_ready * next_node = NULL;
    node_ready * toRemove = NULL;

    if(*head == NULL){
        return NULL;
    }


    int min_burst = INT_MAX; 

    while((*head) != NULL){
        if((*head)->val->burst < min_burst){
            min_burst = (*head)->val->burst;
            toRemove = (*head);
        }
        head = toRemove->next;
    }

    next_node = toRemove->next;
    retval = toRemove->val;
    free(toRemove);
    toRemove = next_node;

    return retval;


}


void* job_scheduler(void* client_Socket);
void* consult_queue(void* queue);
void* timeG();
void* fifo ();
void * RR (void* quantum);
void changemode(int);
int  kbhit(void);

int check(int exp, const char* mjs);
int kbhit (void)
{
  struct timeval tv;
  fd_set *rdfs;
  rdfs = (fd_set*)malloc(sizeof(fd_set));
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  FD_ZERO(rdfs);
  FD_SET (STDIN_FILENO, rdfs);
  select(STDIN_FILENO+1, rdfs, NULL, NULL, &tv);  
  int returnValue = FD_ISSET(STDIN_FILENO, rdfs);    
  return returnValue;
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
    struct PCB toInsert = {1,5,5,0,13};
    ready_queue= push(ready_queue, toInsert);
    struct PCB toInsert2 = {2,3,5,0,12};
    ready_queue= push(ready_queue, toInsert2);
    struct PCB toInsert3 = {3,1,5,0,12};
    ready_queue= push(ready_queue, toInsert3); 
    printf("hola");  
    struct PCB min_burst = *pop_spf(ready_queue);
    //print_list(ready_queue);
    //print_process(pop_spf(ready_queue));
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

void* fifo (){
    struct PCB* p_PCB = pop(ready_queue);   //Pasa a ejecutarse el proceso.
    struct PCB PCB = *((struct PCB*)p_PCB);
    
    pthread_mutex_lock(&lock);
    PCB.tLlegada= TiempoGlobal;           //Se le asigna el tiempo de entrada al procesador
    pthread_mutex_unlock(&lock);
    
    for(int i; i<PCB.burst;i++){           //Ejecuto
            sleep(1);
    }
    pthread_mutex_lock(&lock); 
    PCB.tSalida = TiempoGlobal;           //Se le asigna el tiempo de salida del procesador
    pthread_mutex_unlock(&lock);

    finish_queue = push(finish_queue,PCB); //Se agrega el proceso a la cola de terminados
    cantProcesos++;
}

void * RR (void* quantum){
    struct PCB* p_PCB = pop(ready_queue);   //Pasa a ejecutarse el proceso.
    struct PCB PCB = *((struct PCB*)p_PCB);
    
    char *q = malloc(sizeof(2));
    q=quantum;
    
    pthread_mutex_lock(&lock);
    PCB.tLlegada= TiempoGlobal;           //Se le asigna el tiempo de entrada al procesador
    pthread_mutex_unlock(&lock);
    
    for(int i; i<atoi(q[0]);i++){           //Ejecuto
            sleep(1);
            PCB.burst= PCB.burst-1;            
    }
    if(PCB.burst!=0){
        pthread_mutex_lock(&lock); 
        ready_queue = push(ready_queue,PCB); //reingresa a la cola del ready.
        pthread_mutex_unlock(&lock);
    }else{
        pthread_mutex_lock(&lock); 
        PCB.tSalida = TiempoGlobal;           //Se le asigna el tiempo de salida del procesador
        pthread_mutex_unlock(&lock);

        finish_queue = push(finish_queue,PCB); //Se agrega el proceso a la cola de terminados
        cantProcesos++;
    }
    
}
/*
void* SPF (){
    struct PCB* p_PCB = pop_spf(ready_queue);   //Pasa a ejecutarse el proceso.
    struct PCB PCB = *((struct PCB*)p_PCB);
    
    pthread_mutex_lock(&lock);
    PCB.tLlegada= TiempoGlobal;           //Se le asigna el tiempo de entrada al procesador
    pthread_mutex_unlock(&lock);
    
    for(int i; i<PCB.burst;i++){           //Ejecuto
            sleep(1);
    }
    pthread_mutex_lock(&lock); 
    PCB.tSalida = TiempoGlobal;           //Se le asigna el tiempo de salida del procesador
    pthread_mutex_unlock(&lock);

    finish_queue = push(finish_queue,PCB); //Se agrega el proceso a la cola de terminados
    cantProcesos++;
}*/

void* timeG(){
    pthread_mutex_lock(&lock);
    TiempoGlobal++;
    pthread_mutex_unlock(&lock);
}

void printFinish(){
    printf("Number of executed process: %d\n", cantProcesos); //Cantidad de procesos ejecutados
    //Cantidad de segundos con CPU ocioso. Pendiente... //
    int promTAT =0;
    int promWT =0;
    if(finish_queue != NULL){
        node_ready * current = finish_queue;    
        while (current != NULL) {                            //•Tabla de TAT y WT para los procesos ejecutados
            int TAT= current->val->tSalida - current->val->tLlegada;
            int WT = TAT - current->val->burst;
            promTAT+=TAT;
            promWT+=WT;
            printf("Proceso p%d  TAT:%d WT%d\n",current->val->PID,TAT,WT);
            current = current->next;
        }
                                                            //•Promedio de Waiting Time•Promedio de Turn Around Time  
        printf("Promedio del TAT:%d Promedio del WT%d\n",(promTAT/cantProcesos),(promWT/cantProcesos));
    }
    
      
}

void* consult_queue(void* queue){        
    if (kbhit() && flag){    
    node_ready rq = *((node_ready*)queue);
    print_list(&rq);
    }
    
}