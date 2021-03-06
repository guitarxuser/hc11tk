/*--------------------------------------------------------------------
 *
 *		cserv.c
 *		Polling the /dev/cua1 and write the messages in a message queue
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
char input_buffer[MAX_LINE]; 
char message_buffer[MAX_LINE];
char* buf_run_ptr;
typedef struct 
  {
    long mtyp;
    char nachricht[MAX_LINE];
  }message_q
;

/*******************************************************************************

  FUNCTION: receive_string
  descr: is getting a string from the /dev/cua0  and put it to the message queue 

*********************************************************************************/

int receive_string(int fd)
{
  size_t  nbytes;       /* Number of bytes read */
  size_t nbytes_per_read_seq; 
  char buffer[MAX_LINE]={'0'};

  char *bufptr ;      /* Current char in buffer */
  char *input_buf_ptr;
  char *pos_e_x;
  char *ptr;
  int max_count=0;
  FILE* fd_exc;
  char *name_fd_exc ="exch_data";

  int server_kennung ;
  int long leng;
  int comp_erg;
  long msg_len;
  char *msg_len_str_ptr;
  char msg_len_str[10]={'0'};
  message_q msq_to_send;
  input_buf_ptr=input_buffer;
  bufptr=buffer;


  server_kennung = msgget(SERVER_KEY,0666);
  if ( server_kennung  ==-1)
 
    fprintf(stderr,"open server queue failed comtk.c \n");
 
    nbytes = read(fd, bufptr, sizeof(buffer));
  
  if(nbytes >0)
      {
       strncat(input_buf_ptr,bufptr,nbytes);
       buf_run_ptr=buf_run_ptr+nbytes;      /*let run the pointer to the end of buffer*/
      }

    printf("nbytes=%d\n   buf_run_ptr=%p input_buf_ptr =%p\n",(int)nbytes,buf_run_ptr,input_buf_ptr);
    leng=strlen("e_x");
    pos_e_x=strstr(bufptr, "e_x");
  if(pos_e_x){
  //if((*(buf_run_ptr-leng-1) == 'e') && (*(buf_run_p)tr-leng)== '_') && (*(buf_run_ptr-leng+1)=='x'))
  //{
	  msg_len_str_ptr=msg_len_str;
	 strncpy(msg_len_str_ptr,pos_e_x-4,4);
	 msg_len = strtol(msg_len_str_ptr, &ptr, (int)10);
    *(input_buf_ptr+msg_len)='\0';/* cut the e_x part*/
    strcpy(message_buffer,input_buf_ptr);  
    printf("message_buffer=%s\n",message_buffer);
    msq_to_send.mtyp=1;
    strcpy(msq_to_send.nachricht,message_buffer);
    /*reset the ptr to begin of the buffer*/
    printf("server_kennung ist %d\n",server_kennung);
    if(msgsnd(server_kennung,&msq_to_send,MAX_LINE,IPC_NOWAIT)==-1){
      fprintf(stderr,"msgsnd failed\n");
      return(1);
    }
  }
    *input_buf_ptr='\0'; /*reset the input buffer*/
    buf_run_ptr=input_buf_ptr;

 return (0);
}
/******S I G P O L L E R ************/

  static  void receive_string_poller(int) ;
  static  void  sig_alrm(int);
/********  M A I N  ************/
/*                             */
 /******************************/

  int main(int argc, char* argv[])
  {

  	char* device=argv[1];

  	if (argc <2)
  	{
  		fprintf(stderr,"please call ./cserv </dev/pts/number>\n");
  		return(-1);
  	}

	  int    mod_answ;
	  struct termios options;


	  int pid;
	  int server_kennung;
	  int speed;

	  /* open the port */
	  buf_run_ptr=input_buffer;
	  fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	  fcntl(fd,F_SETOWN,getpid());
	  fcntl(fd, F_SETFL, O_ASYNC);

	  /* get the current options */
	  tcgetattr(fd, &options);

	  /* set raw input, 1 second timeout */
	  options.c_cflag     |= (CLOCAL | CREAD);
	  options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
	  options.c_oflag     &= ~OPOST;
	  options.c_iflag     |= IGNCR;
	  options.c_cc[VMIN]  = 1; /*1 byte*/
	  options.c_cc[VTIME] = 0;/* no timeout*/

	  options.c_ispeed=B9600;		/* input speed */
	  options.c_ospeed=B9600;		/* output speed */
	  /* set the options */
	  cfsetospeed(&options,B9600);
	  tcsetattr(fd, TCSANOW, &options);
	  speed=cfgetospeed(&options);
	  printf("baud rate = %d\n",speed);
	  /*Process the communication with message queue
  {es*/

	  if ( server_kennung=(msgget(SERVER_KEY,IPC_CREAT|0666 )) ==-1){
		  fprintf(stderr,"open server queue failed comtk.c\n");}
	  printf("server_kennung=%d\n",server_kennung);
	  for(;;){
		  if(signal(SIGIO,receive_string_poller)==SIG_ERR){
			  fprintf(stderr,"Sig handler failed\n");
		  }
		  pause();
	  }
  } /* Main */

  static  void receive_string_poller(int signo)
  {
	  printf("receive_string poller in action\n");
	  if(signal(SIGIO,receive_string_poller)==SIG_ERR){
		  fprintf(stderr,"Sig handler failed\n");
	  }

	  receive_string(fd);
	  return;
  }

  static void
  sig_alrm(int signo)
  {
	  printf("in sig_alrm: \n");
	  return;
  }
