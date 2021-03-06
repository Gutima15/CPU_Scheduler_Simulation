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
    int burstcopia;
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
char quantum[2];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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
        head->val->burstcopia = val.burstcopia;
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
        current->next->val->burstcopia = val.burstcopia;
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

struct PCB *remove_by_index(node_ready ** head, int n) {
    int i = 0;
    struct PCB *retval = NULL;
    if(*head != NULL){
        node_ready * current = *head;
        node_ready * temp_node = NULL;

        if (n == 0) {
            return pop(head);
        }

        for (i = 0; i < n-1; i++) {
            if (current->next == NULL) {
                return NULL; //habia -1
            }
            current = current->next;
        }

        temp_node = current->next;
        retval = temp_node->val;
        current->next = temp_node->next;
        free(temp_node);

        return retval;
    }
}


int smallestBurstIndex(node_ready *head){
    int min = INT_MAX;
    int index = 0;
    int val = -1;
    if(head != NULL){
        node_ready * current = head;
        while (current != NULL) {
            if(current->val->burst < min){
                val = index;
                //printf("val en la iteracion %d es %d: ", index, val);
                min = current->val->burst;
            }
            index++;
            current = current->next;
        }
    }
    return val;
}

int biggestPriorityIndex(node_ready *head){
    int min = INT_MAX;
    int index = 0;
    int val = -1;
    if(head != NULL){
        node_ready * current = head;
        while (current != NULL) {
            if(current->val->prioridad < min){
                val = index;
                //printf("val en la iteracion %d es %d: ", index, val);
                min = current->val->prioridad;
            }
            index++;
            current = current->next;
        }
    }
    return val;
}

int biggestExitTimeIndex(node_ready *head){
    int max = INT_MIN;
    int index = 0;
    int val = -1;
    if(head != NULL){
        node_ready * current = head;
        while (current != NULL) {
            if(current->val->tSalida > max){
                val = index;
                //printf("val en la iteracion %d es %d: ", index, val);
                max = current->val->tSalida;
            }
            index++;
            current = current->next;
        }
    }
    return val;
}

void* job_scheduler(void* client_Socket);
void* consult_queue();
void* timeG();
void* fifo ();
void* SJF();
void* HPF();
void * RR ();
void changemode(int);
int  kbhit(void);
void printFinish();
int check(int exp, const char* mjs);

int kbhit(){
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

void nonblock(int state)
{
    struct termios ttystate;

    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);

    if (state==1)
    {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    }
    else if (state==0)
    {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

}

void printFinish(){
    if (cantProcesos == 0){
        printf("Theres was no process registered as finished...\n");
    }else{
        printf("Number of executed process: %d\n", cantProcesos); //Cantidad de procesos ejecutados
        int biggest= biggestExitTimeIndex(finish_queue);        
        int idleCPU = TiempoGlobal-biggest;    //Cantidad de segundos con CPU ocioso.//
        printf("Idle CPU time: %d\n", idleCPU); //Cantidad de procesos ejecutados
        int promTAT =0;
        int promWT =0;
        if(finish_queue != NULL){
            node_ready * current = finish_queue;    
            while (current != NULL) {                            //•Tabla de TAT y WT para los procesos ejecutados
                int TAT= current->val->tSalida - current->val->tLlegada;
                int WT = TAT - current->val->burst;
                promTAT+=TAT;
                promWT+=WT;
                printf("Proceso p%d  TAT:%d WT:%d\n",current->val->PID,TAT,WT);
                current = current->next;
            }
                                                                //•Promedio de Waiting Time•Promedio de Turn Around Time  
            printf("Promedio del TAT:%d Promedio del WT:%d\n",(promTAT/cantProcesos),(promWT/cantProcesos));
        }
    }

}

int check(int exp, const char *mjs){
    if(exp == SOCKETEROR){
        perror(mjs);
        exit(1);
    }
    return exp;
}

int main() 
{ 
    /******Menú de selección del algoritmo*******/
    char answer[2];

    int validateAlgorithm = getLine("1.FIFO\n2.SJF\n3.HPF\n4.Round Robin.\nSelect the number of scheduler algorithm: ",answer,sizeof(answer));
    if(validateAlgorithm == 1){
        puts("You have no select anything, try again.");
        return 0;
    }
    if(validateAlgorithm == 2){
        puts("Your input is too long, try again.");
        return 0;
    }
    if (answer[0]!='1' && answer[0]!='2' && answer[0]!='3' && answer[0]!='4'){
        puts("That is an invalid number, please try again.");
        return 0;
    }
    if(answer[0]=='4'){
        int q= getLine("Select the quantum of the algorithm: ",quantum,sizeof(quantum));
        if(q == 1){
        puts("You have no select anything, try again.");
        return 0;
        }
        if(q == 2){
            puts("Your input is too long, try again.");
            return 0;
        }
    }
    
    printf("You have select the option #%s\n\n",answer);

    /******lógica de conexión al del servidor*******/
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
    
    char c;
    int i=0;
    /******Ciclo principal*******/
    nonblock(1);
    while(!i){
        usleep(1);
        i=kbhit();
        if (i!=0)
        {
            c=fgetc(stdin);
            if (c=='c'){
                consult_queue(ready_queue);
                i=0;
            } if (c=='q'){
                i=1;                
            }            
        }
        printf("waiting for connections\n");
        //wait for, and eventually accept an incoming connection
        addr_size = sizeof(SA_IN);
        check(client_socket = 
                accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size),
                "Accept failed");
        printf("Connected\n");    
        
        /**********Lógica del CPU***********/       

        /******Job scheduler*******/       
        pthread_t t;
        int*pclient = malloc(sizeof(int));
        *pclient = client_socket;        
        pthread_create(&t,NULL,job_scheduler,pclient);
        
        /******Tiempo global*******/
        pthread_t time;
        pthread_create(&time,NULL,timeG,NULL);

        /******Job scheduler*******/       
        
        if(answer[0] == '1'){
            pthread_t t_fifo;
            pthread_create(&t_fifo,NULL,fifo,NULL);
        }
        if(answer[0] == '2'){
            pthread_t t_SJF;
            pthread_create(&t_SJF,NULL,SJF,NULL);            
        }
        if(answer[0] == '3'){
            pthread_t t_HPF;
            pthread_create(&t_HPF,NULL,HPF,NULL);
        }
        if(answer[0] == '4'){
            pthread_t t_RR;
            pthread_create(&t_RR,NULL,RR,quantum);
        }

    }
    nonblock(0);
    /******Impresión final*******/
    printFinish();
    return 0;
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
        struct PCB p_c_b = {PID, (int)(burst) , (int)(priority) ,TiempoGlobal,0 ,(int)(burst)};           
        //Agregarla a la cola
        pthread_mutex_lock(&lock);
        ready_queue = push(ready_queue,p_c_b);        
        pthread_mutex_unlock(&lock);
        bzero(buffer, sizeof(buffer)); // limpiamos el buffer
		sprintf(buffer,"%d",PID);
        PID++;
        write(client_Socket,buffer,sizeof(buffer));   
        close(client_Socket); 
        printf("closing connection\n");
        
    }     
    return NULL;
    
}

void* fifo (){
    if(ready_queue != NULL){
        pthread_mutex_lock(&lock);
        struct PCB* p_PCB = pop(&ready_queue);   //Pasa a ejecutarse el proceso.
        struct PCB PCB = *((struct PCB*)p_PCB);        
        PCB.tLlegada= TiempoGlobal;           //Se le asigna el tiempo de entrada al procesador
        pthread_mutex_unlock(&lock);
        printf("PID #%d with burst: %d and priority:%d is going to be executed\n\n",PCB.PID, PCB.burst,PCB.prioridad);
        pthread_mutex_lock(&lock);
        for(int i=0; i<PCB.burst;i++){           //Ejecuto
                sleep(1);
                
                TiempoGlobal++;
                
        }
        pthread_mutex_unlock(&lock);

        pthread_mutex_lock(&lock); 
        PCB.tSalida = TiempoGlobal;           //Se le asigna el tiempo de salida del procesador
        pthread_mutex_unlock(&lock);
        printf("The process PID#%d arrived in:%d, finished at:%d, with burst:%d and priority:%d have finished...\n\n",PCB.PID,PCB.tLlegada,PCB.tSalida, PCB.burst,PCB.prioridad);
        finish_queue = push(finish_queue,PCB); //Se agrega el proceso a la cola de terminados
        cantProcesos++;
    }
    
}

void * RR (){    
    if(ready_queue != NULL){
        pthread_mutex_lock(&lock); 
        struct PCB* p_PCB = pop(&ready_queue);   //Pasa a ejecutarse el proceso.        
        struct PCB PCB = *((struct PCB*)p_PCB);
        pthread_mutex_unlock(&lock); 
        
        if(PCB.tLlegada == 0){
            pthread_mutex_lock(&lock);
            PCB.tLlegada= TiempoGlobal;           //Se le asigna el tiempo de entrada al procesador
            pthread_mutex_unlock(&lock);
        }
        printf("PID #%d with burst: %d and priority:%d is going to be executed\n\n",PCB.PID, PCB.burst,PCB.prioridad);    
        printf("PID #%d with burstcopia: %d Antes de ejecutar\n",PCB.PID,PCB.burstcopia);    
        
        pthread_mutex_lock(&lock); 
                        //5                      3
        for(int i=0; i<atoi(quantum);i++){           //Ejecuto                
                if(PCB.burstcopia ==0){
                    break;
                }
                sleep(1);
                PCB.burstcopia--;
                TiempoGlobal++;           
        }
        printf("PID #%d with burstcopia: %d luego de ejecutar\n",PCB.PID,PCB.burstcopia);    
        pthread_mutex_unlock(&lock);

        if(PCB.burstcopia!=0){
            pthread_mutex_lock(&lock);             
            ready_queue = push(ready_queue,PCB); //reingresa a la cola del ready.
            printf("PID #%d with burstcopia: %d PUSHEADO\n",PCB.PID,PCB.burstcopia);    
            pthread_mutex_unlock(&lock);
        }else{
            pthread_mutex_lock(&lock); 
            PCB.tSalida = TiempoGlobal;           //Se le asigna el tiempo de salida del procesador
            pthread_mutex_unlock(&lock);
            printf("The process PID#%d arrived in:%d, finished at:%d, with burst:%d and priority:%d have finished...\n\n",PCB.PID,PCB.tLlegada,PCB.tSalida, PCB.burst,PCB.prioridad);
            finish_queue = push(finish_queue,PCB); //Se agrega el proceso a la cola de terminados
            cantProcesos++;
        }
    }
}

void* SJF (){
    if(ready_queue != NULL){
        pthread_mutex_lock(&lock);
        int index = smallestBurstIndex(ready_queue);
        struct PCB* p_PCB = remove_by_index(&ready_queue, index);   //Pasa a ejecutarse el proceso.
        struct PCB PCB = *((struct PCB*)p_PCB);
        pthread_mutex_unlock(&lock);       
        
        pthread_mutex_lock(&lock);
        PCB.tLlegada= TiempoGlobal;           //Se le asigna el tiempo de entrada al procesador
        pthread_mutex_unlock(&lock);

        printf("PID #%d with burst: %d and priority:%d is going to be executed\n\n",PCB.PID, PCB.burst,PCB.prioridad);
        pthread_mutex_lock(&lock); 
        for(int i=0; i<PCB.burst;i++){           //Ejecuto
                sleep(1);
                TiempoGlobal++;
        }
        pthread_mutex_unlock(&lock); 

        pthread_mutex_lock(&lock); 
        PCB.tSalida = TiempoGlobal;           //Se le asigna el tiempo de salida del procesador
        pthread_mutex_unlock(&lock);
        
        printf("The process PID#%d arrived in:%d, finished at:%d, with burst:%d and priority:%d have finished...\n\n",PCB.PID,PCB.tLlegada,PCB.tSalida, PCB.burst,PCB.prioridad);
        finish_queue = push(finish_queue,PCB); //Se agrega el proceso a la cola de terminados
        cantProcesos++;
        }    
}

void* HPF (){
    if(ready_queue != NULL){
        pthread_mutex_lock(&lock);
        int index = biggestPriorityIndex(ready_queue);        
        struct PCB* p_PCB = remove_by_index(&ready_queue,index);   //Pasa a ejecutarse el proceso.
        struct PCB PCB = *((struct PCB*)p_PCB);
        pthread_mutex_unlock(&lock);

        pthread_mutex_lock(&lock);
        PCB.tLlegada= TiempoGlobal;           //Se le asigna el tiempo de entrada al procesador
        pthread_mutex_unlock(&lock);
        
        printf("PID #%d with burst: %d and priority:%d is going to be executed\n\n",PCB.PID, PCB.burst,PCB.prioridad);
        pthread_mutex_lock(&lock); 
        for(int i=0; i<PCB.burst;i++){           //Ejecuto
                sleep(1);
                TiempoGlobal++;
        }
        pthread_mutex_unlock(&lock); 

        pthread_mutex_lock(&lock); 
        PCB.tSalida = TiempoGlobal;           //Se le asigna el tiempo de salida del procesador
        pthread_mutex_unlock(&lock);

        printf("The process PID#%d arrived in:%d, finished at:%d, with burst:%d and priority:%d have finished...\n\n",PCB.PID,PCB.tLlegada,PCB.tSalida, PCB.burst,PCB.prioridad);
        finish_queue = push(finish_queue,PCB); //Se agrega el proceso a la cola de terminados
        cantProcesos++;
    }
}

void* timeG(){
    pthread_mutex_lock(&lock);
    TiempoGlobal++;
    pthread_mutex_unlock(&lock);
}

void* consult_queue(){  
    pthread_mutex_lock(&lock);
    print_list(ready_queue);
    pthread_mutex_unlock(&lock);
    
}