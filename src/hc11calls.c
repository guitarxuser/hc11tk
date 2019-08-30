/*--------------------------------------------------------------------
 *
 *		HC11CALLS.C
 *		In this module are the modules wich support the hc11 calls
 *
 *--------------------------------------------------------------------*/
#include "hc11global.h"
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>

#define ENUF(msg, value) { perror(msg); exit(value); }
#define TRUE 0
#define FALSE -1


/* status could be TRUE or FALSE ,*/
/*--------------------------------*/ 
/*----------------------------------------------*/
/*This function sets up the serial I/O          */
/*----------------------------------------------*/
void set_up_the_serial_dev(int fd)
{
      struct termios options;
    /* open the port */
    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd <0)
    {
    	fd = open("/dev/ttyUSB1", O_RDWR | O_NOCTTY | O_NDELAY);
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
    options.c_cc[VMIN]  = 0; /*"\n\n\000ing-buero kerber  v1.1!"*/
    options.c_cc[VTIME] = 10;

    /* set the options */
    tcsetattr(fd, TCSANOW, &options); 
}

/*----------------------------------------------*/
/*This function filters    the null char        */
/*----------------------------------------------*/
void null_filt(char *inp_buff)
{

  int i,n = 0;
  while(n <= 1000) /*so far as the buffer length is overflow protected*/
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
out_message init_modem_low_level()   /* I - Serial port file */
{
  char buffer[26]; /*Input buffer */
  char *bufptr;      /* Current char in buffer */
  int  nbytes;       /* Number of bytes read */
  int  tries;        /* Number of tries so far */
  int n=0;
  offset_message get_offset; /* input structure for offset calculation*/
  out_message modem_response; 
/*---------------*/
  /*  set_up_the_serial_dev(fd);*/
    struct termios options;
    /* open the port */
    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd <0)
    {
    	 fd = open("/dev/ttyUSB1", O_RDWR | O_NOCTTY | O_NDELAY);
    	 if (fd <0)
    	 {
          strcpy(modem_response.buffer,"DEVICE NOT CONNECTED");
   	      modem_response.status=FALSE;
   	      return modem_response;
    	 }
    }

    fcntl(fd, F_SETFL, 0);

    /* get the current options */
    tcgetattr(fd, &options);

    /* set raw input, 1 second timeout */
    options.c_cflag     |= (CLOCAL | CREAD|CRTSCTS);
    options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag     &= ~OPOST;
    options.c_cc[VMIN]  = 26; /*"\n\n\000ing-buero kerber  v1.1!"*/
    options.c_cc[VTIME] = 10;/*10 x 0.1 s = 1 sec*/
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    /* set the options */
    tcsetattr(fd, TCSANOW, &options); 

  /* read characters into our string buffer until we get a CR or NL */

      bufptr = buffer;   
       
      while(nbytes = read(fd, bufptr,26) > 0)
	{
	  if (nbytes == 0)
	    {
	      strcpy(modem_response.buffer,"DEVICE NOT CONNECTED");
	      modem_response.status=FALSE;
	      return modem_response;
	    }
	  if (buffer[25] == '!')
	    {
	      break;
	    }
	}
      get_offset=count_offset_len(bufptr);
      if (get_offset.status == FALSE) 
        {
          strcpy(modem_response.buffer,"WRONG OFFSET");
	  modem_response.status=FALSE;
	  return modem_response;
	}
      while(bufptr[n+get_offset.offset_length]!='!')
	{
	  bufptr[n]=bufptr[n+get_offset.offset_length];
	  n++;
          if (n > 26) /*the last character '!' is not sended*/
	    { 
	      strcpy(modem_response.buffer,"LAST CHAR FAIL");
	      modem_response.status=FALSE;
	      return modem_response;
	    }
	}
      bufptr[n]= '\0';
      if (strncmp(buffer, "ing-buero kerber  v1.1", 25) == 0)
	{
          strcpy(modem_response.buffer,buffer);
          modem_response.status=TRUE;
          write(fd, "\r", 1);
	      read(fd, bufptr,4);
          return modem_response;
	}

  return modem_response;
}

/*This function is reading the assigned memory area by the command MD ....*/
out_message  hc11_cmd_md(int fd,const char* command)
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
  strcat (full_command_ptr,"\x0D");
  bufptr = buffer;
  ioctl(fd, TIOCMGET,&serial);
  ioctl(fd, FIONREAD,&serial);
 // write(fd,"\x0D", 1);
  command_length = write(fd,full_command_ptr, strlen(full_command_ptr));
  ioctl(fd, TIOCMGET,&serial);
  ioctl(fd, FIONREAD,&serial);
  nbytes = read(fd, bufptr, sizeof(buffer));
  my_delay(5000);
  /*       strcpy(buffer,buffer+3+command_length);*/
  null_filt(buffer);
  strcpy(buffer,buffer+command_length+1);

  strcpy(modem_response.buffer,buffer);
  modem_response.status=TRUE;

  /*  fprintf(stdout,"%s",buffer);*/
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

int my_delay(unsigned long mikros)
  {
  struct timespec ts;
  int resp;

  ts.tv_sec = mikros / 1000000L;
  ts.tv_nsec = (mikros % 1000000L) * 1000L;
  resp = nanosleep(&ts, (struct timespec *)NULL);
  return(resp);
  }
