/*--------------------------------------------------------------------
 *
 *		COMCALLS.C
 *		this file  contains  modules which support the systen 
 *              communication  calls
 *
 *--------------------------------------------------------------------*/
#include "comglobal.h"
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <poll.h>
#include <stropts.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define ENUF(msg, value) { perror(msg); exit(value); }
#define TRUE 0
#define FALSE -1
#define SYN 22 /*Ascii synchr. steruer zeichen*/
#define STX 2 /*text begin*/
#define ETX 3 /*text end*/

/* status could be TRUE or FALSE ,*/
/*--------------------------------*/ 
/*----------------------------------------------*/
/*This function sets up the serial I/O          */
/*----------------------------------------------*/
void set_up_the_serial_dev(int fd)
{
      struct termios options;
    /* open the port */
    fd = open(SERIAL_DEV, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd <0)
        {
        	fd = open(SERIAL_DEV, O_RDWR | O_NOCTTY | O_NDELAY);
        }
    fcntl(fd, F_SETFL, 0);

    /* get the current options */
    tcgetattr(fd, &options);
    /* set baudrate */
      cfsetispeed(&options, B9600);
      cfsetospeed(&options, B9600);
    /* set raw input, 1 second timeout */
    options.c_cflag     |= (CLOCAL | CREAD|CRTSCTS);
    options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag     &= ~OPOST;
    options.c_cc[VMIN]  = 26; /*"\n\n\000ing-buero kerber  v1.1!"*/
    options.c_cc[VTIME] = 1;

    /* set the options */
    tcsetattr(fd, TCSANOW, &options); 
}

/*----------------------------------------------*/
/*This function filters    the null char        */
/*----------------------------------------------*/
void null_filt(char *inp_buff)
{

  int i,n = 0;
  while(n <= 10000) /*so far as the buffer length is overflow protected*/
    {
      if (inp_buff[n] == '\0')
	{
	  inp_buff[n]= ' ';
        }
      n++;
      if (inp_buff[n] == '\n' && inp_buff[n+1] == '\0' && inp_buff[n+2] == '>') break;
    }

}
/*----------------------------------------------*/
/*This function filters    the null char        */
/*----------------------------------------------*/
void change_blank_to_rep(char *inp_buff)
{

  int i,n = 0;
  while(n < strlen(inp_buff)) /*so far as the buffer length is overflow protected*/
    {
      if (inp_buff[n] == ' ')
	{
	  inp_buff[n]= '*';
        }
      n++;
      if (inp_buff[n] == '\0') break;
    }
}
/*----------------------------------------------*/
/*This function calculates the offset to the    */
/*character i.. in the initial message from hc11*/ 
/*controller                                    */
/*----------------------------------------------*/

offset_message count_offset_len(char *input_buff)
{

  int n=0,offset_=0;
  offset_message offset_response;
  while(input_buff[n] != 'i')
    {
      n++;
      if (n > 5) 
	{
          offset_response.offset_length = n;
          offset_response.status = FALSE;
          return offset_response;
	}
    }  
  offset_response.offset_length = n;
  offset_response.status = TRUE; 
  return offset_response;
}

int  hc11_cmd_cr(int fd)
{
  char buffer[10000];        /* Input buffer */
  char out_buffer[10000];
  char *bufptr;            /* Current char in buffer */
  int  nbytes = 0;         /* Number of bytes read */
 
  
  /* send an CR command */


  /* read characters into our string buffer until we get a '>' and '\0' or ' ' */
  bufptr = buffer;
  write(fd,"\r", 1);
  while ((nbytes = read(fd, bufptr, sizeof(buffer))) > 0)
    {
      bufptr += nbytes;
      if (bufptr[-1] == '>' && (bufptr[-2] == '\0' || bufptr[-2] == ' '))
	{
	  break;
	}
    }
  strcpy(buffer,buffer+8);
  fprintf(stdout,"%s",buffer);
  return 0;
 
}

                  /* O - 0 = MODEM ok, -1 = MODEM bad */
out_message init_modem_low_level(int closure)   /* I - Serial port file */
{
  char buffer[64000]; /*Input buffer */
  char *bufptr;      /* Current char in buffer */
  int  nbytes;       /* Number of bytes read */
  int  tries;        /* Number of tries so far */
  int n=0;

  fd_set writeset,readset;    /*dscriptors for reading or writing with select*/
  struct timeval tv;          /*time conditioning for selects*/  

  struct termios options;

  offset_message get_offset; /* input structure for offset calculation*/
  out_message modem_response; 
/*---------------*/
  /*  set_up_the_serial_dev(fd);*/



    FD_ZERO(&writeset);
    FD_ZERO(&readset);
    FD_SET(fd,&writeset);
    FD_SET(fd,&readset);

    /* open the port */
    if(closure == TO_OPEN)
      {
    fd = open(SERIAL_DEV, O_RDWR | O_NOCTTY | O_NDELAY);
    fcntl(fd, F_SETFL, 0);
      
    /* get the current options */
    tcgetattr(fd, &options);
    tcgetattr(fd, &options_old);/*save the previous options for system conformity*/

    /* set raw input, 1 second timeout */
    options.c_cflag     |= (CLOCAL | CREAD|CRTSCTS);
    options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG); /*canonical mode line oriented*/
    options.c_oflag     &= ~OPOST;
    options.c_cc[VMIN]  = 1; /*1 byte*/
    options.c_cc[VTIME] = 0;/*no timer*/
 
    /* set the options */
    tcsetattr(fd, TCSANOW, &options);
      } 
    else
      {
        tcsetattr(fd, TCSANOW, &options_old);
        close(fd);
      }

    strcpy(modem_response.buffer,buffer);
    modem_response.status=TRUE;
    return modem_response;
	
  return modem_response;
}

/*This function is reading the assigned memory area by the command MD ....*/
out_message  hc11_cmd_md(int fd, const char* command)
{
  char buffer[10000];  /* Input buffer */
  char out_buff[10000];
  char command_holder[7]; /*MD nnnn int here shall be placed the fool command*/
  char *bufptr;      /* Current char in buffer */
  int  nbytes;       /* Number of bytes read */
  int  tries;        /* Number of tries so far */
  int n_tick = 0,do_nothing;
  int command_length = 0;
  char *full_command_ptr; /*the full command syntax MD <nnnn> it means memory dump at the assigned address*/
  out_message modem_response;
  /* send an command followed by a CR */
  full_command_ptr=command_holder; 
  strcpy(full_command_ptr,"MD ");
  strcat(full_command_ptr,command);
  bufptr = buffer;
  command_length = write(fd,full_command_ptr, strlen(full_command_ptr));
  write(fd,"\x0D", 1);
  while ((nbytes = read(fd, bufptr, sizeof(buffer))) > 0)
    {
      bufptr += nbytes;
      if (bufptr[-1] == '>' && (bufptr[-2] == '\0' || bufptr[-2] == ' '))
	{
	  break;
	}
      n_tick++;
    }
  /*       strcpy(buffer,buffer+3+command_length);*/
  null_filt(buffer);
  strcpy(buffer,buffer+command_length+1);

  strcpy(modem_response.buffer,buffer);
  modem_response.status=TRUE;

  /*  fprintf
(stdout,"%s",buffer);*/
  return modem_response;
 
}

out_message  hc11_cmd_mm(int fd,const char *memory_address,const char *memory_tag)
{
  char buffer[10000];  /* Input buffer */
  char out_buff[10000];
  char full_command_holder[10]; /*MM XXXX YY here shall be placed the fool command*/
  char *bufptr;      /* Current char in buffer */
  int  nbytes;       /* Number of bytes read */
  int  tries;        /* Number of tries so far */
  int n_tick = 0,do_nothing;
  int command_length = 0;
  char *full_command_ptr; /*the full command syntax MD <nnnn> it means memory dump at the assigned address*/
  out_message modem_response;

  /* send an command followed by a CR */
  full_command_ptr=full_command_holder; 
  strcpy(full_command_ptr,"MM ");/*building the hc11 monitor command MM XXXX YY*/
  strcat(full_command_ptr,memory_address);
  bufptr = buffer;
  command_length = write(fd,full_command_ptr, strlen(full_command_ptr));
  write(fd,"\x0D", 1);
  while ((nbytes = read(fd, bufptr, sizeof(buffer))) > 0)
    {  
      bufptr += nbytes; 
      break; 
    }

  write(fd,memory_tag,strlen(memory_tag));
  write(fd,"\x0D", 1);
  while ((nbytes = read(fd, bufptr, sizeof(buffer))) > 0)
    {  
     bufptr += nbytes;  
     break; 
    }
  strcpy(buffer,buffer+3+command_length);
  fprintf(stdout,"%s",buffer); 
 return modem_response;
}
 /**************************************************************************

  FUNCTION: send_string
  descr: is getting a string from the inp widget and put it to the /dev/cua0

 ***************************************************************************/

out_message  send_string(int fd,char *inp_str)
{
  char *preamble="FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n";
  char *buffer;
  size_t inp_str_len;
  char ascii_length[3];
  char text_begin[2];
  char text_end[2];
  out_message modem_response;

  change_blank_to_rep(inp_str);
  inp_str_len=strlen(inp_str);
  sprintf(text_begin,"%c",STX);
  sprintf(text_end,"%c",ETX);
  sprintf(ascii_length,"%d",(int)inp_str_len);
  fprintf(stderr,"ascii_length=%s\n",ascii_length);
  buffer=malloc((size_t)inp_str_len+3+4096);/*aligned biffer to 4K+3 bytes for strlen(e_x)=3*/
  strcpy(buffer,inp_str);
  strcat(buffer,ascii_length);
  strcat(buffer,"e_x");/*this is the sieg_proc*/
  modem_response.status=write(fd,buffer,strlen(buffer));
  write(fd,"\n",1);  
  fprintf(stderr,"%s\n",buffer); 
  write(fd,text_end,strlen(text_end));
  free(buffer);
 return modem_response;
 
}

/*******************************************************************************

  FUNCTION: receive_string
  descr: is getting a string from the /dev/cua0  and put it to the output widget 

*********************************************************************************/

static  void receive_string_poller(int signo)

{
     int nbytes;
     char *bufptr;
     char buffer[10000];
     extern char receiver_buffer[];

     bufptr=receiver_buffer;
     printf("receive_string poller in action\n");

     if(signal(SIGPOLL,receive_string_poller)==SIG_ERR){
       fprintf(stderr,"Sig handler failed\n");}

     while ((nbytes = read(fd, bufptr, sizeof(buffer))) > 0)
       {	   
         bufptr += nbytes;
         if (bufptr[-1] == '\n' )
	   {
	    break;
	   }
       }
     return; 
}

out_message  receive_string(int fd,char *out_str)

{
  extern char receiver_buffer[];
  out_message modem_response; 
  mqu_answer mqu_answer; 
  int server_kennung; /*message queue identity*/

  server_kennung = msgget(SERVER_KEY,0666);
  if ( server_kennung ==-1)
     fprintf(stderr,"open server queue failed comcalls.c\n");
      if(msgrcv(server_kennung,&mqu_answer,MAX_LINE,0,0)==-1){
	fprintf(stderr,"msgrcv failed\n");
        modem_response.status=FALSE;
      } 
 strcpy(modem_response.buffer,mqu_answer.buffer);
 modem_response.status=TRUE;

 return modem_response;
 
}

int  hc11_cmd(int fd,char* command)
{
  char buffer[10000];  /* Input buffer */
  char out_buff[10000];
  char *bufptr;      /* Current char in buffer */
  char *dummy_ptr;
  int  nbytes;       /* Number of bytes read */
  int  tries;        /* Number of tries so far */
  int n_tick =0 ,do_nothing;
  int command_length = 0;

  /* send an command followed by a CR */
  bufptr = buffer;
  command_length = write(fd,command, strlen(command));
  write(fd,"\x0D", 1);
  /*        read(fd,dummy_ptr,7);*/
  while ((nbytes = read(fd, bufptr, sizeof(buffer))) > 0)
    {
	   
      bufptr += nbytes;
      if (bufptr[-1] == '>' && (bufptr[-2] == '\0' || bufptr[-2] == ' '))
	{
	  break;
	}
    }
  strcpy(buffer,buffer+3+command_length);
  fprintf(stdout,"%s",buffer);
  return 0;
}
