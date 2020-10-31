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
#include <termios.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include<pthread.h>
#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2

/*static int getLine (char *prmpt, char *buff, size_t sz) {
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
*/

void *consultar(){
    char ch;
    while(true)
	{
		printf("Enter any character: ");
		//read a single character
		ch=fgetc(stdin);
		
		if(ch==0x0A)
		{
			printf("ENTER KEY is pressed.\n");
			break;
		}
		else
		{
			printf("\n%c is pressed.\n",ch);
		}

	}
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
int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}
int main()
{
    char c;
    int i=0;

    nonblock(1);
    while(!i)
    {
        sleep(1);
        i=kbhit();
        if (i!=0)
        {
            c=fgetc(stdin);
            if(c=='q'){
                printf("print final queue");                
                i=1;
            }
            if (c=='c'){
                printf("Consult");
                i=0;
            }
            //else{
                
            //}
        }

        fprintf(stderr,"%d ",i);
    }
    printf("\n you hit %c. \n",c);
    nonblock(0);

    return 0;
}

