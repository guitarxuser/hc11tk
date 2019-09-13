/*--------------------------------------------------------------------
 *
 *		RELATED.C
 *		Main Module for LEARN PROGRAM ABOUT THE SERIAL COMMUNICATION
 *
 *    Function: main()
 *
 *	 Usage: main(argc, argv);
 *		int argc;
 *		char *argv[];
 *
 *--------------------------------------------------------------------*/

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdlib.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define ENUF(msg, value) { perror(msg); exit(value); }
#define TRUE 0
#define FALSE -1
#define SERVER_KEY 10001 /*key for message q*/
#define MAX_LINE 256    /* max char for a message*/

/*--------GLOBAL TYPES-----------*/
struct offset_message
{
  int offset_length;
  int status;
};

/*********GLOBAL VARIABLES ************************/
int fd;
 
typedef struct 
  {
    long mtyp;
    char nachricht[MAX_LINE];
  }message_q
;
/* status could be TRUE or FALSE ,*/
/*--------------------------------*/ 
/*----------------------------------------------*/
/*This function calculates the offset to the    */
/*character i.. in the initial message from hc11*/ 
/*controller                                    */
/*----------------------------------------------*/

/*******************************************************************************

  FUNCTION: receive_string
  descr: is getting a string from the /dev/cua0  and put it to the output widget 

*********************************************************************************/

int receive_string(int fd,char *out_str)
{
  int  nbytes;       /* Number of bytes read */
  char buffer[MAX_LINE];
  char *bufptr;      /* Current char in buffer */
  int max_count=0;
  FILE* fd_exc;
  char *name_fd_exc ="exch_data";

  int server_kennung;

  message_q msq_to_send;

  bufptr=buffer;
  server_kennung = msgget(SERVER_KEY,0666);
  if ( server_kennung  ==-1)
  fprintf(stderr,"open server queue failed comtk.c \n");
  nbytes = read(fd, bufptr, sizeof(buffer));
 
  msq_to_send.mtyp=1;
  strcpy(msq_to_send.nachricht,buffer);
  printf("server_kennung ist %d\n",server_kennung);
  if(msgsnd(server_kennung,&msq_to_send,MAX_LINE,IPC_NOWAIT)==-1){
      fprintf(stderr,"msgsnd failed\n");
  }
 return (0);
}
/******S I G P O L L E R ************/

  static  void receive_string_poller(int) ;
  static  void  sig_alrm(int);
/********  M A I N  ************/
/*                             */
 /******************************/

main(argc, argv)

     int argc;
     char *argv[];
{
  int    mod_answ;        
  struct termios options;
  char *bufptr;
  char buffer[10];
  int nbytes;
  int pid;
  int server_kennung;
 
 /* open the port */
  fd = open("/dev/cua0", O_RDWR | O_NOCTTY | O_NDELAY);
  fcntl(fd,F_SETOWN,getpid());
  fcntl(fd, F_SETFL, O_ASYNC);
        
  /* get the current options */
  tcgetattr(fd, &options);

  /* set raw input, 1 second timeout */
  options.c_cflag     |= (CLOCAL | CREAD);
  options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
  options.c_oflag     &= ~OPOST;
  options.c_iflag     |= IGNCR;
  options.c_cc[VMIN]  = 1; /*communication start conditionfor my evaluation board*/
  options.c_cc[VTIME] = 0;
    

  /* set the options */
  tcsetattr(fd, TCSANOW, &options);

  /*Process the communication with message queues*/
  
  if ( server_kennung=(msgget(SERVER_KEY,IPC_CREAT|0666 )) ==-1){
    fprintf(stderr,"open server queue failed comtk.c\n");}
  printf("server_kennung=%d\n",server_kennung);
  for(;;){
  if(signal(SIGPOLL,receive_string_poller)==SIG_ERR){
    fprintf(stderr,"Sig handler failed\n");
    }
  }    
} /* main */

static  void receive_string_poller(int signo)
{
     printf("receive_string poller in action\n");
     if(signal(SIGPOLL,receive_string_poller)==SIG_ERR){
       fprintf(stderr,"Sig handler failed\n");
     }
     receive_string(fd,"TEST");
  return;
}

static void
sig_alrm(int signo)
{
	printf("in sig_alrm: \n");
	return;
}
